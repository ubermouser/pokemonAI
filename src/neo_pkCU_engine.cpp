#include "pokemonai/neo_pkCU_engine.h"

#include <spdlog/spdlog.h>

#include <algorithm>

#include "pokemonai/fp_compare.h"
#include "pokemonai/neo_pkCU.h"
#include "pokemonai/pkCU_types.h"
#include "pokemonai/pluggable_types.h"

#include <map>

static size_t factorial(size_t n) {
  return (n == 0 || n == 1) ? 1 : n * factorial(n - 1);
}

/**
 * @def CALLPLUGIN
 * @brief A macro for invoking plugins of a specific type.
 *
 * This macro iterates through all registered plugins of a given `pluginType`
 * for the current matchup and calls their `pluginFunction`. The return value
 * of each plugin is OR'd with `retValue`. The loop breaks if `retValue`
 * becomes greater than 1, which is a convention to indicate that a plugin has
 * handled the event and no further plugins should be called.
 *
 * @param retValue The variable to store the combined return values of the
 * plugins.
 * @param pluginType The type of plugin to call (e.g., `PLUGIN_ON_MODIFYSPEED`).
 * @param pluginFunction The function signature of the plugin to be called.
 * @param ... The arguments to pass to the plugin function.
 */
#define CALLPLUGIN(retValue, pluginType, pluginFunction, ...)        \
  {                                                                  \
    const std::vector<plugin_t>& cPlugins =                          \
        getCPluginSet()[(size_t)pluginType];                         \
    for (auto iPlugin = cPlugins.cbegin(), iPSize = cPlugins.cend(); \
         iPlugin != iPSize;                                          \
         ++iPlugin) {                                                \
      pluginFunction cPlugin = (pluginFunction)iPlugin->pFunction;   \
      retValue = retValue | cPlugin(__VA_ARGS__);                    \
      if (retValue > 1) { break; }                                   \
    }                                                                \
  }


NeoPkCUEngine::NeoPkCUEngine(
    const NeoPkCU& cu,
    const EnvironmentVolatileData& initial,
    const ActionMap& actions)
    : cu_(cu), cPlugins_(&cu.pluginSet_), actions_(actions), iBase_(0) {
  stack_.setNonvolatileEnvironment(cu.nv_);
  stack_.push_back(EnvironmentPossibleData::create(initial, false));

  StackFrame firstFrame;
  firstFrame.iStack = 0;
  firstFrame.stage = StageType::SEEDED;
  firstFrame.iActor = 0;
  firstFrame.iTarget = 0;
  stackFrame_.push_back(firstFrame);
}


/**
 * @brief The main entry point for the engine, simulating a single turn.
 *
 * This method orchestrates the entire process of a battle turn. It first
 * determines the move priority to decide which Pokemon acts first. It then
 * calls `updateState_move` to process both Pokemon's moves. If there's a
 * speed tie, it creates two separate scenarios, one for each Pokemon moving
 * first. Finally, it evaluates end-of-round effects and combines similar
 * resulting environments.
 */
PossibleEnvironments NeoPkCUEngine::updateState() {
  evaluateMove();

  // combine environments that equal eachother:
  combineSimilarEnvironments();

  return std::move(stack_);
}


uint32_t NeoPkCUEngine::computeSpeed(const Actor& actor) {
  PokemonVolatile pkv = getBase().getEnv().teammate(actor);
  uint32_t speed = pkv.getFV_boosted(FV_SPEED);

  int result = 0;
  CALLPLUGIN(
      result, PLUGIN_ON_MODIFYSPEED, onModifySpeed_rawType, *this, pkv, speed);

  return speed;
}


NeoPkCUEngine::MoveBracket NeoPkCUEngine::computeMoveBracket(
    const Actor& actor) {
  const Action& action = actions_.at(actor);
  int32_t actionBracket = 0;

  if (action.isSwitch()) {
    actionBracket = 6;
  } else if (action.isMove()) {
    PokemonVolatile pkv = getBase().teammate(actor);
    if (action.isStruggle()) {
      actionBracket = 0;
    } else {
      MoveVolatile mv = pkv.getMV(action);
      actionBracket = mv.getBase().getSpeedPriority();

      int result = 0;
      CALLPLUGIN(
          result,
          PLUGIN_ON_SETSPEEDBRACKET,
          onModifyBracket_rawType,
          *this,
          mv,
          pkv,
          actionBracket);
    }
  } else if (action.isWait()) {
    actionBracket = -7;
  }

  uint32_t speed = computeSpeed(actor);
  return {actionBracket, speed};
}


std::unordered_map<Actor, NeoPkCUEngine::MoveBracket>
NeoPkCUEngine::computeMoveBrackets() {
  std::unordered_map<Actor, MoveBracket> results;
  std::vector<Actor> active = getBase().getEnv().getActivePokemon();
  for (const auto& actor : active) {
    results[actor] = computeMoveBracket(actor);
  }
  return std::move(results);
}


std::vector<Actor> NeoPkCUEngine::computeActorOrder() {
  auto brackets = computeMoveBrackets();
  std::vector<Actor> actors = getBase().getEnv().getActivePokemon();

  std::sort(actors.begin(), actors.end(), [&](const Actor& a, const Actor& b) {
    const auto& bracketA = brackets.at(a);
    const auto& bracketB = brackets.at(b);
    if (bracketA.actionBracket != bracketB.actionBracket) {
      return bracketA.actionBracket > bracketB.actionBracket;
    } else if (bracketA.speed != bracketB.speed) {
      return bracketA.speed > bracketB.speed;
    } else {
      // TODO - implement tiebreaking
      return bracketA.tiebreaker > bracketB.tiebreaker;
    }
    
  });


  return std::move(actors);
}


/**
 * @brief Duplicates an environment on the stack to represent two possible
 * outcomes.
 *
 * This function takes a single environment and splits it into two, each with a
 * different probability. This is used to model stochastic events, such as a
 * move hitting or missing.
 *
 * @param result An array to store the indices of the two resulting
 * environments.
 * @param _probability The probability of the second outcome. The probability of
 *        the first outcome is calculated as `1.0 - _probability`.
 * @param iState The index of the environment to duplicate.
 */
void NeoPkCUEngine::duplicateState(
    std::array<size_t, 2>& result, FixType branchProbability, size_t iState) {
  if (iState == SIZE_MAX) { iState = iBase_; }
  if (!mostlyGT(branchProbability, FixType(0))) {
    result[0] = iState;
    result[1] = iState; // Or some other indication of no split
    return;
  }
  assert(mostlyLT(branchProbability, FixType(1)));

  // duplicate state 2 times
  nPlicateState(result, iState);

  // modify probabilities of resulting states:
  FixType totalProbability = getBase(result[0]).getProbability();
  FixType absoluteBranchProb = totalProbability * branchProbability;

  assert(mostlyGT(absoluteBranchProb, FixType(0)));
  assert(mostlyLT(absoluteBranchProb, totalProbability));

  getBase(result[1]).getProbability() = absoluteBranchProb;
  getBase(result[0]).getProbability() = totalProbability - absoluteBranchProb;

  assert(saneStackProbability());
}


/**
 * @brief Duplicates an environment on the stack to represent three possible
 * outcomes.
 *
 * This function is similar to `duplicateState` but creates three environments
 * from a single one. This is used for events with three possible outcomes.
 *
 * @param result An array to store the indices of the three resulting
 * environments.
 * @param _probability The probability of the second outcome.
 * @param _oProbability The probability of the third outcome.
 * @param iState The index of the environment to triplicate.
 */
void NeoPkCUEngine::triplicateState(
    std::array<size_t, 3>& result,
    FixType _probability,
    FixType _oProbability,
    size_t iState) {
  assert(
      _probability > FixType(0) && _oProbability > FixType(0) &&
      (_probability + _oProbability) < FixType(1));

  // duplicate state 3 times
  nPlicateState(result, iState);

  // modify probabilities of resulting states:
  FixType totalProbability = getBase(result[0]).getProbability();
  FixType branchBProbability = totalProbability * _probability;
  FixType branchCProbability = totalProbability * _oProbability;
  FixType remainingProbability =
      totalProbability - branchBProbability - branchCProbability;

  // Branch B
  assert(branchBProbability > FixType(0));
  assert(branchBProbability < (totalProbability - branchCProbability));
  getBase(result[1]).getProbability() = branchBProbability;

  // Branch C
  assert(branchCProbability > FixType(0));
  assert(branchCProbability < (totalProbability - branchBProbability));
  getBase(result[2]).getProbability() = branchCProbability;

  // Branch A (Original)
  assert(remainingProbability > FixType(0));
  getBase(result[0]).getProbability() = remainingProbability;

  assert(saneStackProbability());
}


bool NeoPkCUEngine::saneStackProbability() const {
  FixType sumOfStateProbabilities = FixType(0);
  for (auto begin = stack_.begin(), end = stack_.end(); begin != end; ++begin) {
    auto probability = begin->getProbability();
    if (begin->isPruned()) { continue; }

    sumOfStateProbabilities += probability;
    if (!(probability > FixType(0)) || !(probability <= FixType(1))) {
      return false;
    }
  }

  return sumOfStateProbabilities == FixType(1);
}


void NeoPkCUEngine::evaluateMove() {
  size_t stagesCompleted = 0;
  while (stagesCompleted != getStack().size()) {
    StackFrame& frame = getStackFrame();
    if (!frame.moveOrder.empty() && frame.iActor < frame.moveOrder.size()) {
      Actor& actor = frame.moveOrder.at(frame.iActor);
      SPDLOG_TRACE(
          "STACK={} STAGE={} PKMN={} ACTION={} TARGET={}",
          iBase_,
          stageTypeToString(frame.stage),
          fmt::streamed(actor),
          fmt::streamed(actions_.at(actor)),
          fmt::streamed(targets_.at(actor).at(frame.iTarget)));
    } else {
      SPDLOG_TRACE(
          "STACK={} STAGE={}", iBase_, stageTypeToString(frame.stage));
    }

    switch (frame.stage) {
    case StageType::SEEDED:
      break;
    case StageType::PRETURN:
      evaluateMove_preturn();
      break;
    case StageType::SELECTORDER:
      evaluateMove_selectOrder();
      break;
    case StageType::PRESWITCH:
    case StageType::POSTSWITCH:
      // evaluateMove_switch(); // legacy had evaluateMove_switch, but Neo might
      // handle it differently in the loop for now, let's treat them as needing
      // implementation if they appear in our StageType enum
      break;
    case StageType::STATUS:
      evaluateMove_status();
      break;
    case StageType::MOVEBASE:
      evaluateMove_damage_moveBase();
      break;
    case StageType::MODIFYHITCHANCE:
      evaluateMove_damage_modifyHitChance();
      break;
    case StageType::EVALUATEHITCHANCE:
      evaluateMove_damage_evaluateHitChance();
      break;
    case StageType::MODIFYCRITCHANCE:
      evaluateMove_damage_modifyCritChance();
      break;
    case StageType::EVALUATECRITCHANCE:
      evaluateMove_damage_evaluateCritChance();
      break;
    case StageType::SETBASEPOWER:
      evaluateMove_damage_setBasePower();
      break;
    case StageType::SETMOVETYPE:
      evaluateMove_damage_setMoveType();
      break;
    case StageType::MODIFYBASEPOWER:
      evaluateMove_damage_modifyBasePower();
      break;
    case StageType::MODIFYATTACKPOWER:
      evaluateMove_damage_modifyAttackPower();
      break;
    case StageType::MODIFYCRITICALPOWER:
      evaluateMove_damage_modifyCriticalPower();
      break;
    case StageType::MODIFYRAWDAMAGE:
      evaluateMove_damage_modifyRawDamage();
      break;
    case StageType::MODIFYSTAB:
      evaluateMove_damage_modifySTAB();
      break;
    case StageType::MODIFYTYPERESISTANCE:
      evaluateMove_damage_modifyTypeResistance();
      break;
    case StageType::MODIFYITEMPOWER:
      evaluateMove_damage_modifyItemPower();
      break;
    case StageType::PREDAMAGE:
      evaluateMove_damage_preDamage();
      break;
    case StageType::POSTDAMAGE:
      // evaluateMove_postDamage();
      break;
    case StageType::POSTMOVE:
      evaluateMove_postMove();
      break;
    case StageType::PRESECONDARY:
      evaluateMove_preSecondary();
      break;
    case StageType::MODIFYSECONDARYHITCHANCE:
      evaluateMove_modifySecondaryHitChance();
      break;
    case StageType::SECONDARY:
      evaluateMove_secondary();
      break;
    case StageType::POSTSECONDARY:
      // evaluateMove_postSecondary();
      break;
    case StageType::POSTTURN:
      evaluateMove_postTurn();
      break;
    case StageType::POSTROUND:
      evaluateMove_postRound();
      break;
    case StageType::HASH:
      evaluateMove_round_hash();
      break;
    case StageType::FINAL:
      stagesCompleted += 1;
      break;
    case StageType::DNE:
    default:
      throw std::runtime_error(fmt::format(
          "Unimplemented stackstage at STACK={}: {}-{}!",
          frame.iStack,
          (int32_t)frame.stage,
          stageTypeToString(frame.stage)));
    }


    // advance stack stage of current state
    advanceStackStage();
    iBase_ = (iBase_ + 1) % getStack().size();
  }  // endOf while stages not completed
}  // endOf evaluateMove


void NeoPkCUEngine::evaluateMove_preturn() {
  const Action& cAction = getCAction();
  size_t iCTeam = getICTeam();

  if (cAction.isMove()) {
    int result = 0;
    // CALLPLUGIN(result, PLUGIN_ON_MODIFYACTION, ...); // Implementation
    // details omitted for brevity
  }
}


void NeoPkCUEngine::evaluateMove_selectOrder() {
  StackFrame& frame = getStackFrame();

  // 1. Compute MoveBrackets if not already done
  if (frame.moveBrackets.empty()) {
    std::vector<Actor> active = getBase().getEnv().getActivePokemon();
    for (const auto& actor : active) {
      frame.moveBrackets[actor] = computeMoveBracket(actor);
    }
  }

  // 2. Identify ties (actors with same actionBracket and speed, and NO
  // tiebreaker)
  std::map<std::pair<int32_t, uint32_t>, std::vector<Actor>> tieGroups;
  for (const auto& pair : frame.moveBrackets) {
    const Actor& actor = pair.first;
    const MoveBracket& bracket = pair.second;
    if (bracket.tiebreaker == 0) {  // Assuming 0 means not yet disambiguated
      tieGroups[{bracket.actionBracket, bracket.speed}].push_back(actor);
    }
  }

  // Find the first group with N > 1
  for (auto& group : tieGroups) {
    if (group.second.size() > 1) {
      std::vector<Actor>& tiedActors = group.second;
      size_t N = tiedActors.size();
      size_t numOutcomes = factorial(N);

      std::vector<size_t> indices;
      nPlicateStateDynamic(indices, numOutcomes);

      FixType baseProb = getBase().getProbability();
      FixType outcomeProb = baseProb / (int32_t)numOutcomes;

      // Ensure stable sorting for permutation
      std::sort(
          tiedActors.begin(), tiedActors.end(), [](const Actor& a, const Actor& b) {
            if (a.iTeam() != b.iTeam()) return a.iTeam() < b.iTeam();
            return a.iTeammate() < b.iTeammate();
          });

      size_t iPerm = 0;
      do {
        size_t iIdx = indices[iPerm++];
        getBase(iIdx).getProbability() = outcomeProb;
        for (size_t i = 0; i < N; ++i) {
          stackFrame_[iIdx].moveBrackets[tiedActors[i]].tiebreaker =
              static_cast<uint32_t>(N - i);
        }
        // Counteract the advanceStackStage in the main loop to re-evaluate this group or the next
        stackFrame_[iIdx].stage =
            static_cast<StageType>(StageType::SELECTORDER - 1);
      } while (std::next_permutation(
          tiedActors.begin(), tiedActors.end(), [](const Actor& a, const Actor& b) {
            if (a.iTeam() != b.iTeam()) return a.iTeam() < b.iTeam();
            return a.iTeammate() < b.iTeammate();
          }));

      return;  // Handled one tie, will re-evaluate in the next pass
    }
  }

  // 3. If no more ties, construct moveOrder
  std::vector<Actor> actors;
  for (const auto& pair : frame.moveBrackets) {
    actors.push_back(pair.first);
  }

  std::sort(actors.begin(), actors.end(), [&](const Actor& a, const Actor& b) {
    const auto& bracketA = frame.moveBrackets.at(a);
    const auto& bracketB = frame.moveBrackets.at(b);
    if (bracketA.actionBracket != bracketB.actionBracket) {
      return bracketA.actionBracket > bracketB.actionBracket;
    } else if (bracketA.speed != bracketB.speed) {
      return bracketA.speed > bracketB.speed;
    } else {
      return bracketA.tiebreaker > bracketB.tiebreaker;
    }
  });

  frame.moveOrder = std::move(actors);
}


void NeoPkCUEngine::evaluateMove_status() {
  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_BEGINNINGOFTURN,
      onBeginningOfTurn_rawType,
      *this,
      getPKV());
}


void NeoPkCUEngine::evaluateMove_damage_moveBase() {
  const Move& cMove = getMV().getBase();
  getDamageComponent().category = cMove.getDamageType();
}


void NeoPkCUEngine::evaluateMove_damage_modifyHitChance() {
  FixType& probabilityToHit = getDamageComponent().tProbability;
  probabilityToHit = getProbabilityToHit();

  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYHITPROBABILITY,
      onModifyProbability_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      probabilityToHit);
}


void NeoPkCUEngine::evaluateMove_damage_evaluateHitChance() {
  FixType& probabilityToHit = getDamageComponent().tProbability;
  probabilityToHit =
      std::max(std::min(probabilityToHit, FixType(1)), FixType(0));

  if (mostlyGT(probabilityToHit, FixType(0))) {
    if (mostlyLT(probabilityToHit, FixType(1))) {
      std::array<size_t, 2> iHEnv;
      duplicateState(iHEnv, (FixType(1) - probabilityToHit));
      getStack().at(iHEnv[1]).setHit(getICTeam()); // wait this is wrong
      stackFrame_[iHEnv[1]].stage = static_cast<StageType>(stackFrame_[iBase_].stage + 1);
    }
    getBase().setHit(getICTeam());
  } else {
    // miss
    stackFrame_[iBase_].stage = StageType::POSTDAMAGE;
  }
}


void NeoPkCUEngine::evaluateMove_damage_modifyCritChance() {
  FixType& probabilityToCrit = getDamageComponent().tProbability;
  probabilityToCrit = getPKV().getAccuracy_boosted(FV_CRITICALHIT);

  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYCRITPROBABILITY,
      onModifyProbability_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      probabilityToCrit);
}


void NeoPkCUEngine::evaluateMove_damage_evaluateCritChance() {
  FixType& probabilityToCrit = getDamageComponent().tProbability;
  probabilityToCrit =
      std::max(std::min(probabilityToCrit, FixType(1)), FixType(0));

  if (mostlyGT(probabilityToCrit, FixType(0))) {
    if (mostlyLT(probabilityToCrit, FixType(1))) {
      std::array<size_t, 2> iCEnv;
      duplicateState(iCEnv, probabilityToCrit);
      getStack().at(iCEnv[1]).setCrit(getICTeam());
    }
    // base continues normally (non-crit or handled by duplicate)
  }
}


void NeoPkCUEngine::evaluateMove_damage_setBasePower() {
  uint32_t& basePower = getDamageComponent().damage;
  basePower = getMV().getBase().getPower();

  int result = (basePower != UINT8_MAX) ? 1 : 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_SETBASEPOWER,
      onSetPower_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      basePower);
}


void NeoPkCUEngine::evaluateMove_damage_setMoveType() {
  const Type*& cType = getDamageComponent().mType;
  cType = &getMV().getBase().getType();

  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_SETMOVETYPE,
      onModifyMoveType_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      cType);
}


void NeoPkCUEngine::evaluateMove_damage_modifyBasePower() {
  uint32_t& basePower = getDamageComponent().damage;
  fpType baseModifier = 1.0;

  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYBASEPOWER,
      onModifyPower_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      baseModifier);

  basePower = (uint32_t)(basePower * baseModifier);
}


void NeoPkCUEngine::evaluateMove_damage_modifyAttackPower() {
  PokemonVolatile cPKV = getPKV();
  PokemonVolatile tPKV = getTPKV();
  const Move& cMove = getMV().getBase();
  DamageComponents_t& cDamage = getDamageComponent();

  size_t attackType, defenseType;
  if (cMove.getDamageType() == ATK_PHYSICAL) {
    attackType = FV_ATTACK;
    defenseType = FV_DEFENSE;
  } else {
    attackType = FV_SPATTACK;
    defenseType = FV_SPDEFENSE;
  }

  fpType attackPowerModifier = 1.0;
  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYATTACKPOWER,
      onModifyPower_rawType,
      *this,
      getMV(),
      cPKV,
      tPKV,
      attackPowerModifier);

  uint32_t attackPower = cPKV.getFV_boosted(attackType);
  uint32_t defensePower = tPKV.getFV_boosted(defenseType);
  uint32_t levelModifier = ((cPKV.nv().getLevel() * 2) / 5) + 2;

  if (getBase().hasCrit(getICTeam())) {
    attackPower = std::max(cPKV.nv().getFV_base(attackType), attackPower);
    defensePower = std::min(tPKV.nv().getFV_base(defenseType), defensePower);
  }

  cDamage.damage =
      ((levelModifier * cDamage.damage * attackPower) / 50) / defensePower;
  cDamage.damage = (uint32_t)(cDamage.damage * attackPowerModifier) + 2;
}


void NeoPkCUEngine::evaluateMove_damage_modifyCriticalPower() {
  if (getBase().hasCrit(getICTeam())) {
    fpType criticalHitModifier = 2.0;
    int result = 0;
    CALLPLUGIN(
        result,
        PLUGIN_ON_MODIFYCRITICALPOWER,
        onModifyPower_rawType,
        *this,
        getMV(),
        getPKV(),
        getTPKV(),
        criticalHitModifier);
    getDamageComponent().damage =
        (uint32_t)(getDamageComponent().damage * criticalHitModifier);
  }
}


void NeoPkCUEngine::evaluateMove_damage_preDamage() {
  if (getBase().hasHit(getICTeam())) { calculateDamage(); }
}


void NeoPkCUEngine::evaluateMove_postMove() {
  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_ENDOFMOVE,
      onEvaluateMove_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV());
}


void NeoPkCUEngine::evaluateMove_preSecondary() {
  const Move& cMove = getMV().getBase();
  if (!getPKV().isAlive() || !(cMove.getSecondaryAccuracy() > FixType(0))) {
    stackFrame_[iBase_].stage = StageType::POSTTURN;
    return;
  }

  FixType& secondaryHitProbability = getDamageComponent().tProbability;
  secondaryHitProbability = cMove.getSecondaryAccuracy();

  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYSECONDARYPROBABILITY,
      onModifyProbability_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      secondaryHitProbability);
}


void NeoPkCUEngine::evaluateMove_modifySecondaryHitChance() {
  FixType& secondaryHitProbability = getDamageComponent().tProbability;
  secondaryHitProbability =
      std::max(std::min(secondaryHitProbability, FixType(1)), FixType(0));

  if (getBase().hasHit(getICTeam()) &&
      mostlyGT(secondaryHitProbability, FixType(0))) {
    if (mostlyLT(secondaryHitProbability, FixType(1))) {
      std::array<size_t, 2> iREnv;
      duplicateState(iREnv, (FixType(1) - secondaryHitProbability));
    }
    getBase().setSecondary(getICTeam());
  } else {
    stackFrame_[iBase_].stage = StageType::POSTSECONDARY;
  }
}


void NeoPkCUEngine::evaluateMove_secondary() {
  if (getBase().hasSecondary(getICTeam())) {
    int result = 0;
    CALLPLUGIN(
        result,
        PLUGIN_ON_SECONDARYEFFECT,
        onEvaluateMove_rawType,
        *this,
        getMV(),
        getPKV(),
        getTPKV());
  }
}


void NeoPkCUEngine::evaluateMove_postTurn() {
  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_ENDOFTURN, onEndOfTurn_rawType, *this, getPKV());

  StackFrame& frame = getStackFrame();
  std::vector<Actor>& targets = targets_[frame.moveOrder[frame.iActor]];

  // increment the stack stage:
  if (frame.iTarget < targets.size()) {
    frame.iTarget += 1;
    frame.stage = StageType::PRETURN;
  } else if (frame.iTarget == targets.size()) {
    if (frame.iActor < actions_.size()) {
      frame.iActor += 1;
      frame.iTarget = 0;
      frame.stage = StageType::PRETURN;
    } else {
      advanceStackStage();
    }
  }
}


void NeoPkCUEngine::evaluateMove_postRound() {
  int result = 0;
  CALLPLUGIN(
      result, PLUGIN_ON_ENDOFROUND, onEndOfRound_rawType, *this, getPKV());
}


void NeoPkCUEngine::evaluateMove_round_hash() {
  getBase().data().generateHash();
}


void NeoPkCUEngine::evaluateMove_damage_modifyRawDamage() {
  DamageComponents_t& cDamage = getDamageComponent();

  /* Mod2 = Other modifier
    1.3 if item = life orb
    1+.1*n if item = metronome and used the same move n previous times, to a max
    of n=10 1.5 if attacking with Me First and attacks first (NOTE: SPECIAL
    BEHAVIOR with life orb / metronome!) 1 else
    */
  fpType rawDamageMultiplier = 1.0;
  auto mv = getMV();
  auto pkv = getPKV();
  auto tpkv = getTPKV();
  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYRAWDAMAGE,
      onModifyPower_rawType,
      *this,
      mv,
      pkv,
      tpkv,
      rawDamageMultiplier);

  // incorporate raw damage modifier:
  cDamage.damage = (uint32_t)(cDamage.damage * rawDamageMultiplier);
}


void NeoPkCUEngine::evaluateMove_damage_modifySTAB() {
  PokemonVolatile cPKV = getPKV();
  DamageComponents_t& cDamage = getDamageComponent();

  bool hasStab =
      ((&cPKV.getBase().getType(0) == cDamage.mType) ||
       (&cPKV.getBase().getType(1) == cDamage.mType));
  fpType STABMultiplier = hasStab ? 1.5 : 1.0;
  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYSTAB,
      onModifyPower_rawType,
      *this,
      getMV(),
      cPKV,
      getTPKV(),
      STABMultiplier);

  // incorporate STAB modifier:
  cDamage.damage = (uint32_t)(cDamage.damage * STABMultiplier);
}


void NeoPkCUEngine::evaluateMove_damage_modifyTypeResistance() {
  PokemonVolatile tPKV = getTPKV();
  DamageComponents_t& cDamage = getDamageComponent();

  fpType typeModifier = 1.0;
  {
    // type1:
    typeModifier *= cDamage.mType->getModifier(tPKV.getBase().getType(0));
    // type 2:
    typeModifier *= cDamage.mType->getModifier(tPKV.getBase().getType(1));
  }
  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_SETDEFENSETYPE,
      onModifyTypePower_rawType,
      *this,
      *cDamage.mType,
      getMV(),
      getPKV(),
      getTPKV(),
      typeModifier);

  // incorporate type modifier:
  cDamage.damage = (uint32_t)(cDamage.damage * typeModifier);
}


void NeoPkCUEngine::evaluateMove_damage_modifyItemPower() {
  DamageComponents_t& cDamage = getDamageComponent();

  /* Mod3 = SRF × EB × TL × TRB */
  fpType itemModifier = 1.0;
  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYITEMPOWER,
      onModifyPower_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      itemModifier);

  // incorporate item modifier:
  cDamage.damage = (uint32_t)(cDamage.damage * itemModifier);
}


void NeoPkCUEngine::updateState_move() {
  throw std::runtime_error("NeoPkCUEngine::updateState_move not implemented");
}


int32_t NeoPkCUEngine::movePriority_Bracket() {
  throw std::runtime_error(
      "NeoPkCUEngine::movePriority_Bracket not implemented");
}


uint32_t NeoPkCUEngine::movePriority_Speed() {
  throw std::runtime_error("NeoPkCUEngine::movePriority_Speed not implemented");
}


/**
 * @brief Combines environments on the stack that are identical.
 *
 * After all the branching from stochastic events, the stack may contain
 * multiple environments that are in the same state. This function identifies
 * these duplicates by hashing each environment and then merging the ones with
 * the same hash. The probabilities of the merged environments are summed up.
 * This is a crucial optimization to keep the number of possible environments
 * manageable.
 *
 * @return The number of unique environments remaining on the stack.
 */
size_t NeoPkCUEngine::combineSimilarEnvironments() {
  PossibleEnvironments& stack = getStack();

  // hash environments (and summate probabilities for check):
  for (iBase_ = 0; iBase_ != stack.size(); ++iBase_) {
    EnvironmentPossible cEnvironment = getBase();

    // assert that each of these environments is getting hashed:
    assert(getStackStage() == StageType::FINAL);

    cEnvironment.data().generateHash();
  }

  size_t iSize = stack.size();
  std::unordered_map<uint64_t, size_t> envMap;
  envMap.reserve(iSize);

  // compare environment hashes:
  for (size_t iEnv = 0; iEnv != iSize; iEnv++) {
    EnvironmentPossible cEnv = stack.at(iEnv);

    // don't attempt to merge pruned environments
    if (cEnv.isPruned()) { continue; }

    uint64_t hash = cEnv.getHash();
    auto it = envMap.find(hash);

    if (it != envMap.end()) {
      // Found a duplicate! Merge into the existing environment
      size_t existIndex = it->second;
      EnvironmentPossible existEnv = stack.at(existIndex);

#ifdef _PKCUCHECKSIGNATURE
      // Assert that same hash implies same environment data
      assert((oEnv.hash == iEnv.hash) == (oEnv.env == iEnv.env));
#endif

      // combine the two environments by adding their probabilities
      existEnv.getProbability() += cEnv.getProbability();

      // this is probably not representative of the current environment now
      existEnv.getBitmask() &= cEnv.getBitmask();

      // flag the destination environment as merged
      existEnv.setMerged();

      // flag the current environment as pruned
      cEnv.setPruned();

      // decrement number of unique values in vector
      stack.decrementUnique();
    } else {
      // First time seeing this hash, add to map
      envMap[hash] = iEnv;
    }
  }

  // Calculate accumulated probability for verification
  assert(saneStackProbability());
  return stack.getNumUnique();
}  // endOf combineSimilarEnvironments


uint32_t NeoPkCUEngine::movePriority() {
  throw std::runtime_error("NeoPkCUEngine::movePriority not implemented");
}


void NeoPkCUEngine::calculateDamage() {
  SPDLOG_ERROR("NeoPkCUEngine::calculateDamage not implemented");
  getDamageComponent().damage = 10;
}


FixType NeoPkCUEngine::getProbabilityToHit() {
  SPDLOG_ERROR("NeoPkCUEngine::getProbabilityToHit not implemented");
  return getMV().getBase().getPrimaryAccuracy();
}


void NeoPkCUEngine::duplicateState(
    std::array<size_t, 2>& result, fpType probability, size_t iState) {
  duplicateState(result, FixType(probability), iState);
}



void NeoPkCUEngine::nPlicateStateDynamic(
    std::vector<size_t>& result, size_t numEnvironments, size_t iState) {
  if (iState == SIZE_MAX) { iState = iBase_; }
  PossibleEnvironments& stack = getStack();

  result.resize(numEnvironments);
  result[0] = iState;
  const auto& baseFrame = stackFrame_[iState];
  for (size_t iEnvironment = 1; iEnvironment < numEnvironments;
       ++iEnvironment) {
    size_t cSize = stack.size();

    result[iEnvironment] = cSize;
    stackFrame_.push_back(baseFrame);
    stackFrame_.back().iStack = cSize;

    stack.push_back(stack[iState]);
  }
}


const PluginSet& NeoPkCUEngine::getCPluginSet() {
  if (cPlugins_) { return *cPlugins_; }
  throw std::runtime_error("NeoPkCUEngine::getCPluginSet: cPlugins_ is null");
}


void NeoPkCUEngine::setCPluginSet() {
  throw std::runtime_error("NeoPkCUEngine::setCPluginSet not implemented");
}

void NeoPkCUEngine::advanceStackStage() {
  auto& frame = getStackFrame();
  if (frame.stage < StageType::FINAL) {
    frame.stage = static_cast<StageType>(frame.stage + 1);
  }
}


TeamVolatile NeoPkCUEngine::getTV() { return getTV(iBase_); }


TeamVolatile NeoPkCUEngine::getTTV() { return getTTV(iBase_); }


TeamVolatile NeoPkCUEngine::getTV(size_t iState) {
  return getBase(iState).getTeam(getICTeam());
}


TeamVolatile NeoPkCUEngine::getTTV(size_t iState) {
  return getBase(iState).getTeam(getIOTeam());
}


PokemonVolatile NeoPkCUEngine::getPKV() { return getPKV(iBase_); }


PokemonVolatile NeoPkCUEngine::getTPKV() { return getTPKV(iBase_); }


PokemonVolatile NeoPkCUEngine::getPKV(size_t iState) {
  const StackFrame& frame = stackFrame_[iState];
  if (frame.iActor < frame.moveOrder.size()) {
    const Actor& actor = frame.moveOrder[frame.iActor];
    return getBase(iState).teammate(actor);
  }
  throw std::runtime_error("NeoPkCUEngine::getPKV: no active actor");
}


PokemonVolatile NeoPkCUEngine::getTPKV(size_t iState) {
  return getBase(iState).getTeam(getIOTeam()).getPKV();
}


MoveVolatile NeoPkCUEngine::getMV() { return getMV(iBase_); }


MoveVolatile NeoPkCUEngine::getTMV() { return getTMV(iBase_); }


MoveVolatile NeoPkCUEngine::getMV(size_t iState) {
  const StackFrame& frame = stackFrame_[iState];
  if (frame.iActor < frame.moveOrder.size()) {
    const Actor& actor = frame.moveOrder[frame.iActor];
    return getBase(iState).teammate(actor).getMV(actions_.at(actor));
  }
  throw std::runtime_error("NeoPkCUEngine::getMV: no active actor");
}


MoveVolatile NeoPkCUEngine::getTMV(size_t iState) {
  // Target move is not usually needed in damage calc unless for specific plugins
  // For now, return first move of target
  return getBase(iState).getTeam(getIOTeam()).getPKV().getMV(0);
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent(size_t iStack) {
  const StackFrame& frame = stackFrame_[iStack];
  if (frame.iActor < frame.moveOrder.size()) {
    const Actor& actor = frame.moveOrder[frame.iActor];
    return stackFrame_[iStack].damageComponents[actor];
  }
  throw std::runtime_error("NeoPkCUEngine::getDamageComponent: no active actor");
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent(
    size_t iStack, const Actor& actor) {
  return stackFrame_[iStack].damageComponents.at(actor);
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent(
    size_t iStack, size_t iTeam) {
  for (auto& pair : stackFrame_[iStack].damageComponents) {
    if (pair.first.iTeam() == iTeam) { return pair.second; }
  }
  throw std::runtime_error(
      "NeoPkCUEngine::getDamageComponent: no actor for team " +
      std::to_string(iTeam));
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent() {
  return getDamageComponent(iBase_);
}


const DamageComponents_t& NeoPkCUEngine::getDamageComponent() const {
  const StackFrame& frame = getStackFrame();
  if (frame.iActor < frame.moveOrder.size()) {
    return frame.damageComponents.at(frame.moveOrder[frame.iActor]);
  }
  throw std::runtime_error("NeoPkCUEngine::getDamageComponent: no active actor");
}


size_t NeoPkCUEngine::getICTeam() const {
  const StackFrame& frame = getStackFrame();
  if (frame.iActor < frame.moveOrder.size()) {
    return frame.moveOrder[frame.iActor].iTeam();
  }
  throw std::runtime_error("NeoPkCUEngine::getICTeam: no active actor");
}


size_t NeoPkCUEngine::getIOTeam() const {
  return getICTeam() == TEAM_A ? TEAM_B : TEAM_A;
}


const Action& NeoPkCUEngine::getCAction() const {
  const StackFrame& frame = getStackFrame();
  if (frame.iActor < frame.moveOrder.size()) {
    return actions_.at(frame.moveOrder[frame.iActor]);
  }
  throw std::runtime_error("NeoPkCUEngine::getCAction: no active actor");
}


const Action& NeoPkCUEngine::getOAction() const {
  const StackFrame& frame = getStackFrame();
  if (frame.iActor < frame.moveOrder.size()) {
    const Actor& currentActor = frame.moveOrder[frame.iActor];
    for (const auto& actor : frame.moveOrder) {
      if (actor.iTeam() != currentActor.iTeam()) {
        return actions_.at(actor);
      }
    }
  }
  throw std::runtime_error("NeoPkCUEngine::getOAction not implemented");
}
