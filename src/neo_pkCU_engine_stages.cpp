#include "pokemonai/neo_pkCU_engine.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <map>
#include <stdexcept>

#include "pokemonai/fp_compare.h"
#include "pokemonai/neo_pkCU.h"
#include "pokemonai/pkCU_types.h"
#include "pokemonai/pluggable_types.h"


static size_t factorial(size_t n) {
  return (n == 0 || n == 1) ? 1 : n * factorial(n - 1);
}


void NeoPkCUEngine::evaluateMove() {
  size_t stagesCompleted = 0;
  while (stagesCompleted != getStack().size()) {
    EnvironmentPossible base = getBase();
    StackFrame& frame = getStackFrame();
    if (!frame.moveOrder.empty() && frame.iActor < frame.moveOrder.size()) {
      Actor& actor = frame.moveOrder.at(frame.iActor);
      SPDLOG_TRACE(
          "iSTACK={:4d} P={:.5f} PKMN={} ACT={} TGT={} STAGE={:25s}",
          iBase_,
          base.getProbability().to_double(),
          fmt::streamed(actor),
          fmt::streamed(frame.actions.at(actor)),
          fmt::streamed(frame.targets.at(actor).at(frame.iTarget)),
          stageTypeToString(frame.stage));
    } else {
      SPDLOG_TRACE(
          "iSTACK={:4d} P={:.5f} STAGE={:25s}",
          iBase_,
          base.getProbability().to_double(),
          stageTypeToString(frame.stage));
    }

    switch (frame.stage) {
    case StageType::SEEDED:
      break;
    case StageType::COMPUTEBRACKET:
      evaluateMove_computeBracket();
      break;
    case StageType::POSTCOMPUTEBRACKET:
      evaluateMove_postComputeBracket();
      break;
    case StageType::SELECTORDER:
      evaluateMove_selectOrder();
      break;
    case StageType::PRETURN:
      evaluateMove_preturn();
      break;
    case StageType::PRESWITCH:
      evaluateMove_switch_onSwitchOut();
      break;
    case StageType::POSTSWITCH:
      evaluateMove_switch_onSwitchIn();
      break;
    case StageType::ONBEGINNINGOFTURN:
      evaluateMove_damage_onBeginningOfTurn();
      break;
    case StageType::MOVEBASE:
      evaluateMove_damage_moveBase();
      break;
    case StageType::MODIFYACTION:
      evaluateMove_modifyAction();
      break;
    case StageType::VALIDATEFORCEDACTION:
      evaluateMove_validateForcedAction();
      break;
    case StageType::MODIFYHITCHANCE:
      evaluateMove_damage_modifyHitChance();
      break;
    case StageType::EVALUATEHITCHANCE:
      evaluateMove_damage_evaluateHitChance();
      break;
    case StageType::DAMAGINGMOVEBASE:
      evaluateMove_damage_damagingMoveBase();
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
      evaluateMove_damage_postDamage();
      break;
    case StageType::STATUSMOVE:
      evaluateMove_status_moveBase();
      break;
    case StageType::POSTSTATUSMOVE:
      evaluateMove_status_postMove();
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
    case StageType::EVALSECONDARYHITCHANCE:
      evaluateMove_evaluateSecondaryHitChance();
      break;
    case StageType::SECONDARY:
      evaluateMove_secondary();
      break;
    case StageType::ENDOFTURN:
      evaluateMove_endOfTurn();
      break;
    case StageType::POSTTURN:
      evaluateMove_postTurn();
      break;
    case StageType::ENDOFROUND:
      evaluateMove_endOfRound();
      break;
    case StageType::POSTROUND:
      evaluateMove_postRound();
      break;
    case StageType::HASH:
      evaluateMove_round_hash();
      break;
    case StageType::FINAL:
      // DO NOTHING
      break;
    case StageType::DNE:
    default:
      throw std::runtime_error(fmt::format(
          "Unimplemented stackstage at STACK={}: {}-{}!",
          frame.iStack,
          (int32_t)frame.stage,
          stageTypeToString(frame.stage)));
    }

    maybeCollapseStages();
    // advance stack stage of current state, as well as all new states
    stagesCompleted += advanceAllStages();
    iBase_ = (iBase_ + 1) % getStack().size();
  }  // endOf while stages not completed
}  // endOf evaluateMove


void NeoPkCUEngine::evaluateMove_preturn() {
  const Action& cAction = getCAction();

  // dispatch to appropriate stage:
  if (cAction.isMove()) {
    gotoStackStage(StageType::ONBEGINNINGOFTURN);
  } else if (cAction.isSwitch()) {
    gotoStackStage(StageType::PRESWITCH);
  } else if (cAction.isWait()) {
    getBase().flagsFor(getCActor()).setWaited();
    gotoStackStage(StageType::POSTTURN);
  } else {
    throw std::runtime_error(fmt::format(
        "Unhandled move type {}!", fmt::streamed(cAction)));
  }
}


void NeoPkCUEngine::evaluateMove_computeBracket() {
  const Actor& actor = getCActor();
  getStackFrame().moveBrackets[actor] = computeMoveBracket(actor);
}


void NeoPkCUEngine::evaluateMove_postComputeBracket() {
  StackFrame& frame = getStackFrame();
  size_t nextActor = frame.iActor += 1;
  if (nextActor < frame.moveOrder.size()) {
    frame.iActor = nextActor;
    gotoStackStage(StageType::COMPUTEBRACKET);
  } else {
    frame.iActor = 0;
    gotoStackStage(StageType::SELECTORDER);
  }
}


void NeoPkCUEngine::evaluateMove_selectOrder() {
  StackFrame& frame = getStackFrame();

  // 1. Brackets are already computed in COMPUTEBRACKET stage

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
        gotoStackStage(iIdx, StageType::SELECTORDER);
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
  getBase().flagsFor(frame.moveOrder[0]).setMovedFirst();
}


void NeoPkCUEngine::evaluateMove_modifyAction() {
  const Actor& actor = getCActor();
  Action& action = getStackFrame().actions.at(actor);

  int result = 0;
  result = callPlugins<onModifyAction_rawType>(
      PLUGIN_ON_MODIFYACTION, *this, action);
}


void NeoPkCUEngine::evaluateMove_validateForcedAction() {
  const Actor& actor = getCActor();
  Action& originalAction = actions_.at(actor);
  Action& action = getStackFrame().actions.at(actor);

  if (action == originalAction) { return; }

  auto validation = cu_.isValidAction(getBase().getEnv(), actor, action);
  // forced action is not valid - pokemon must struggle.
  if (!validation) { action = Action::struggle(); }

  // If the plugin changed the action, we must validate it.
  getStackFrame().targets[actor] =
      computeMoveTarget(getBase().getEnv(), actor, action);

  // original action was preempted
  getBase().flagsFor(actor).setBlocked();
}


void NeoPkCUEngine::evaluateMove_damage_onBeginningOfTurn() {
  int result = 0;
  result = callPlugins<onBeginningOfTurn_rawType>(
      PLUGIN_ON_BEGINNINGOFTURN, *this, getPKV());
}


void NeoPkCUEngine::evaluateMove_damage_moveBase() {
  // was this move blocked by a status?
  if (getBase().flagsFor(getCActor()).isBlocked()) {
    gotoStackStage(StageType::ENDOFTURN);
    // did this pokemon die from the last pokemon's action?
  } else if (!getPKV().isAlive()) {
    gotoStackStage(StageType::POSTTURN);
  }
}


void NeoPkCUEngine::evaluateMove_damage_modifyHitChance() {
  FixType& probabilityToHit = getDamageComponent().tProbability;
  const Move& cMove = getMV().getBase();
  if (cMove.targetsEnemy()) {
    probabilityToHit = getProbabilityToHit();
  } else {
    probabilityToHit = FixType(1);
  }

  int result = 0;
  result = callPlugins<onModifyProbability_rawType>(
      PLUGIN_ON_MODIFYHITPROBABILITY, *this, getMV(), getPKV(), getTPKV(), probabilityToHit);
}


void NeoPkCUEngine::evaluateMove_damage_evaluateHitChance() {
  FixType& probabilityToHit = getDamageComponent().tProbability;

  probabilityToHit =
      std::max(std::min(probabilityToHit, FixType(1)), FixType(0));

  SPDLOG_TRACE(
      "iSTACK={:4d} STAGE={:25s} HITPROB={:.4f}",
      iBase_,
      stageTypeToString(getStackStage()),
      probabilityToHit.to_double());

  // hit chance is probabilitistic
  if (mostlyGT(probabilityToHit, FixType(0))) {
    if (mostlyLT(probabilityToHit, FixType(1))) {
      std::array<size_t, 2> iHEnv =
          duplicateState((FixType(1) - probabilityToHit));

      auto hitEnv = getStack().at(iHEnv[0]);
      auto& missFrame = getStackFrame(iHEnv[1]);

      hitEnv.flagsFor(getCActor()).setHit();
      gotoStackStage(missFrame.iStack, StageType::ENDOFTURN);
    } else {  // hits 100% of the time
      getBase().flagsFor(getCActor()).setHit();
    }
  } else {
    // misses 100% of the time
    gotoStackStage(StageType::ENDOFTURN);
  }
}


void NeoPkCUEngine::evaluateMove_damage_damagingMoveBase() {
  const Move& cMove = getMV().getBase();

  // Set the damage category for plugins (like Counter/Mirror Coat) to use
  auto damageType = cMove.getDamageType();
  getDamageComponent().category = damageType;

  // moves that are not physical or special attacks skip all damage computation
  // stages:
  if (damageType != ATK_PHYSICAL && damageType != ATK_SPECIAL) {
    gotoStackStage(StageType::STATUSMOVE);
  }
  // else, follow-through:
}


void NeoPkCUEngine::evaluateMove_status_moveBase() {
  assert(getBase().flagsFor(getCActor()).isHit());
  const Move& cMove = getMV().getBase();

  int result = cMove.isImplemented() ? 1 : 0;
  result = callPlugins<onEvaluateMove_rawType>(
      PLUGIN_ON_EVALUATEMOVE, *this, getMV(), getPKV(), getTPKV());
}


void NeoPkCUEngine::evaluateMove_status_postMove() {
  gotoStackStage(StageType::POSTMOVE);
}


void NeoPkCUEngine::evaluateMove_damage_modifyCritChance() {
  FixType& probabilityToCrit = getDamageComponent().tProbability;
  probabilityToCrit = getPKV().getAccuracy_boosted(FV_CRITICALHIT);

  int result = 0;
  result = callPlugins<onModifyProbability_rawType>(
      PLUGIN_ON_MODIFYCRITPROBABILITY, *this, getMV(), getPKV(), getTPKV(), probabilityToCrit);
}


void NeoPkCUEngine::evaluateMove_damage_evaluateCritChance() {
  FixType& probabilityToCrit = getDamageComponent().tProbability;

  probabilityToCrit =
      std::max(std::min(probabilityToCrit, FixType(1)), FixType(0));

  SPDLOG_TRACE(
      "iSTACK={:4d} STAGE={:25s} CRITPROB={:.4f}",
      iBase_,
      stageTypeToString(getStackStage()),
      probabilityToCrit.to_double());

  // crit chance is probabilistic
  if (mostlyGT(probabilityToCrit, FixType(0))) {
    if (mostlyLT(probabilityToCrit, FixType(1))) {
      std::array<size_t, 2> iCEnv = duplicateState(probabilityToCrit);

      auto critEnv = getStack().at(iCEnv[1]);
      auto& critFrame = getStackFrame(iCEnv[1]);
      critEnv.flagsFor(getCActor()).setCrit();
    } else {  // crits 100% of the time
      getBase().flagsFor(getCActor()).setCrit();
    }
  } else {
    // crit 0% of the time (continues normally)
  }
}


void NeoPkCUEngine::evaluateMove_damage_setBasePower() {
  uint32_t& basePower = getDamageComponent().damage;
  basePower = getMV().getBase().getPower();

  int result = (basePower != UINT8_MAX) ? 1 : 0;
  result = callPlugins<onSetPower_rawType>(
      PLUGIN_ON_SETBASEPOWER, *this, getMV(), getPKV(), getTPKV(), basePower);
}


void NeoPkCUEngine::evaluateMove_damage_setMoveType() {
  const Type*& cType = getDamageComponent().mType;
  cType = &getMV().getBase().getType();

  int result = 0;
  result = callPlugins<onModifyMoveType_rawType>(
      PLUGIN_ON_SETMOVETYPE, *this, getMV(), getPKV(), getTPKV(), cType);
}


void NeoPkCUEngine::evaluateMove_damage_modifyBasePower() {
  uint32_t& basePower = getDamageComponent().damage;
  fpType baseModifier = 1.0;

  int result = 0;
  result = callPlugins<onModifyPower_rawType>(
      PLUGIN_ON_MODIFYBASEPOWER, *this, getMV(), getPKV(), getTPKV(), baseModifier);

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
  result = callPlugins<onModifyPower_rawType>(
      PLUGIN_ON_MODIFYATTACKPOWER, *this, getMV(), cPKV, tPKV, attackPowerModifier);

  uint32_t attackPower = cPKV.getFV_boosted(attackType);
  uint32_t defensePower = tPKV.getFV_boosted(defenseType);
  uint32_t levelModifier = ((cPKV.nv().getLevel() * 2) / 5) + 2;

  if (getBase().flagsFor(getCActor()).isCrit()) {
    attackPower = std::max(cPKV.nv().getFV_base(attackType), attackPower);
    defensePower = std::min(tPKV.nv().getFV_base(defenseType), defensePower);
  }

  cDamage.damage =
      ((levelModifier * cDamage.damage * attackPower) / 50) / defensePower;
  cDamage.damage = (uint32_t)(cDamage.damage * attackPowerModifier) + 2;
}


void NeoPkCUEngine::evaluateMove_damage_modifyCriticalPower() {
  // TODO: use gotoStackStage so we don't need to check for hasCrit
  if (getBase().flagsFor(getCActor()).isCrit()) {
    fpType criticalHitModifier = 2.0;
    int result = 0;
    result = callPlugins<onModifyPower_rawType>(
        PLUGIN_ON_MODIFYCRITICALPOWER, *this, getMV(), getPKV(), getTPKV(), criticalHitModifier);
    getDamageComponent().damage =
        (uint32_t)(getDamageComponent().damage * criticalHitModifier);
  }
}


void NeoPkCUEngine::evaluateMove_damage_preDamage() {
  assert(getBase().flagsFor(getCActor()).isHit());

  calculateDamage();
}


void NeoPkCUEngine::evaluateMove_damage_postDamage() {
  gotoStackStage(StageType::POSTMOVE);
}


void NeoPkCUEngine::evaluateMove_postMove() {
  // skip rest of turn if pokemon has fainted
  if (!getPKV().isAlive()) {
    gotoStackStage(StageType::POSTTURN);
    return;
  }

  callPlugins<onEvaluateMove_rawType>(
      PLUGIN_ON_ENDOFMOVE, *this, getMV(), getPKV(), getTPKV());
}


void NeoPkCUEngine::evaluateMove_preSecondary() {
  const Move& cMove = getMV().getBase();
  // skip end-of-turn action, pokemon fainted:
  if (!getPKV().isAlive()) {
    gotoStackStage(StageType::POSTTURN);
    // no secondary effect occurs:
  } else if (!(cMove.getSecondaryAccuracy() > FixType(0))) {
    gotoStackStage(StageType::ENDOFTURN);
  }
}


void NeoPkCUEngine::evaluateMove_modifySecondaryHitChance() {
  assert(getBase().flagsFor(getCActor()).isHit());

  const Move& cMove = getMV().getBase();
  FixType& probabilityToSecondary = getDamageComponent().tProbability;
  probabilityToSecondary = cMove.getSecondaryAccuracy();

  int result = 0;
  result = callPlugins<onModifyProbability_rawType>(
      PLUGIN_ON_MODIFYSECONDARYPROBABILITY, *this, getMV(), getPKV(), getTPKV(), probabilityToSecondary);
}


void NeoPkCUEngine::evaluateMove_evaluateSecondaryHitChance() {
  assert(getBase().flagsFor(getCActor()).isHit());

  FixType& probabilityToSecondary = getDamageComponent().tProbability;

  probabilityToSecondary =
      std::max(std::min(probabilityToSecondary, FixType(1)), FixType(0));

  SPDLOG_TRACE(
      "iSTACK={:4d} STAGE={:25s} SECPROB={:.4f}",
      iBase_,
      stageTypeToString(getStackStage()),
      probabilityToSecondary.to_double());

  // probabilistic chance of secondary:
  if (mostlyGT(probabilityToSecondary, FixType(0))) {
    if (mostlyLT(probabilityToSecondary, FixType(1))) {
      std::array<size_t, 2> iREnv =
          duplicateState((FixType(1) - probabilityToSecondary));
      gotoStackStage(iREnv[1], StageType::ENDOFTURN);
    }
    // secondary occurs 100% of the time:
    getBase().flagsFor(getCActor()).setSecondary();
  } else {  // secondary occurs 0% of the time:
    gotoStackStage(StageType::ENDOFTURN);
  }
}


void NeoPkCUEngine::evaluateMove_secondary() {
  if (getBase().flagsFor(getCActor()).isSecondary()) {
    int result = 0;
    result = callPlugins<onEvaluateMove_rawType>(
        PLUGIN_ON_SECONDARYEFFECT, *this, getMV(), getPKV(), getTPKV());
  }
}


void NeoPkCUEngine::evaluateMove_endOfTurn() {
  // post-turn action:
  callPlugins<onEndOfTurn_rawType>(PLUGIN_ON_ENDOFTURN, *this, getPKV());
}


void NeoPkCUEngine::evaluateMove_postTurn() {
  StackFrame& frame = getStackFrame();
  std::vector<Actor>& targets = frame.targets[frame.moveOrder[frame.iActor]];

  // if the actor is dead, skip all remaining targets. Move to next actor
  if (!getPKV().isAlive()) { frame.iTarget = targets.size(); }

  // if there are more targets, loop back to pre-turn and execute on the next
  // target:
  size_t nextTarget = frame.iTarget += 1;
  if (nextTarget < targets.size()) {
    frame.iTarget = nextTarget;
    gotoStackStage(StageType::PRETURN);
  } else {
    // if there are more actors, loop back to pre-turn and execute on the next
    // actor:
    size_t nextActor = frame.iActor += 1;
    if (nextActor < frame.moveOrder.size()) {
      frame.iActor = nextActor;
      frame.iTarget = 0;
      gotoStackStage(StageType::PRETURN);
    } else {
      // no more actors or targets. Next stage
      frame.iActor = 0;
      frame.iTarget = 0;
      // allow stackStage to increment at end of postTurn
    }
  }
}


void NeoPkCUEngine::evaluateMove_endOfRound() {
  // do nothing if current actor is not alive, skip to next actor.
  if (!getPKV().isAlive()) { return; }

  // post-round action for current actor:
  callPlugins<onEndOfRound_rawType>(PLUGIN_ON_ENDOFROUND, *this, getPKV());
}


void NeoPkCUEngine::evaluateMove_postRound() {
  // test if other pkmn need to perform their post-round action:
  StackFrame& frame = getStackFrame();
  size_t nextActor = frame.iActor += 1;
  if (nextActor < actions_.size()) {
    frame.iActor = nextActor;
    gotoStackStage(StageType::POSTROUND);
  } else {
    frame.iActor = 0;
    // allow stackStage to increment at end of postRound
  }
}


void NeoPkCUEngine::evaluateMove_round_hash() {
  EnvironmentPossible cEnv = getBase();
  uint64_t hash = cEnv.data().generateHash();

  // we've never seen this state before:
  auto it = stackHashToIdx_.find(hash);
  if (it == stackHashToIdx_.end()) {
    stackHashToIdx_[hash] = iBase_;
    return;
  }
  // merge this duplicate state:
  EnvironmentPossible existEnv = getStack().at(it->second);

  // combine the two environments by adding their probabilities
  existEnv.getProbability() += cEnv.getProbability();

  // merge status flags for visualization
  existEnv.getBitmask() |= cEnv.getBitmask();

  // flag the destination environment as merged
  existEnv.setMerged();

  // flag the current environment as pruned
  cEnv.setPruned();

  // decrement number of unique values in vector
  getStack().decrementUnique();
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
  result = callPlugins<onModifyPower_rawType>(
      PLUGIN_ON_MODIFYRAWDAMAGE,
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
  result = callPlugins<onModifyPower_rawType>(
      PLUGIN_ON_MODIFYSTAB,
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
  result = callPlugins<onModifyTypePower_rawType>(
      PLUGIN_ON_SETDEFENSETYPE,
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
  result = callPlugins<onModifyPower_rawType>(
      PLUGIN_ON_MODIFYITEMPOWER,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      itemModifier);

  // incorporate item modifier:
  cDamage.damage = (uint32_t)(cDamage.damage * itemModifier);
}


void NeoPkCUEngine::evaluateMove_switch_onSwitchOut() {
  int result = 0;
  result = callPlugins<onSwitch_rawType>(PLUGIN_ON_SWITCHOUT, *this, getPKV());
}

void NeoPkCUEngine::evaluateMove_switch_onSwitchIn() {
  Actor switchingActor = getCActor();
  Actor swapTarget = getTarget();
  getTV().swapPokemon(switchingActor, swapTarget);
  getBase().flagsFor(swapTarget).setSwitched();

  handleActorSwitch(switchingActor, swapTarget);

  int result = 0;
  result = callPlugins<onSwitch_rawType>(PLUGIN_ON_SWITCHIN, *this, getPKV());

  gotoStackStage(StageType::ENDOFTURN);
}
