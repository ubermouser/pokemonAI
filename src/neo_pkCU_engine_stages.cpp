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
          fmt::streamed(actions_.at(actor)),
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
    case StageType::STATUS:
      evaluateMove_status();
      break;
    case StageType::MOVEBASE:
      evaluateMove_damage_moveBase();
      break;
    case StageType::EVALUATEHITCHANCE:
      evaluateMove_damage_evaluateHitChance();
      break;
    case StageType::DAMAGINGMOVEBASE:
      evaluateMove_damage_damagingMoveBase();
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
    case StageType::EVALSECONDARYHITCHANCE:
      evaluateMove_evaluateSecondaryHitChance();
      break;
    case StageType::SECONDARY:
      evaluateMove_secondary();
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

    // advance stack stage of current state, as well as all new states
    stagesCompleted += advanceAllStages();
    iBase_ = (iBase_ + 1) % getStack().size();
  }  // endOf while stages not completed
}  // endOf evaluateMove


void NeoPkCUEngine::evaluateMove_preturn() {
  const Action& cAction = getCAction();

  // dispatch to appropriate stage:
  if (cAction.isMove()) {
    gotoStackStage(StageType::MOVEBASE);
  } else if (cAction.isSwitch()) {
    gotoStackStage(StageType::PRESWITCH);
  } else if (cAction.isWait()) {
    getBase().setWaited(getICTeam());
    gotoStackStage(StageType::POSTTURN);
  } else {
    throw std::runtime_error(fmt::format(
        "Unhandled move type {}!", fmt::streamed(cAction)));
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

  // if not a damaging move, skip damage and critical hit stages:
}


void NeoPkCUEngine::evaluateMove_damage_evaluateHitChance() {
  FixType& probabilityToHit = getDamageComponent().tProbability;
  const Move& cMove = getMV().getBase();
  if (cMove.targetsEnemy()) {
    probabilityToHit = getProbabilityToHit();
  } else {
    probabilityToHit = FixType(1);
  }

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

      hitEnv.setHit(getICTeam());
      gotoStackStage(missFrame.iStack, StageType::POSTTURN);
    } else {  // hits 100% of the time
      getBase().setHit(getICTeam());
    }
  } else {
    // misses 100% of the time
    gotoStackStage(StageType::POSTTURN);
  }
}


void NeoPkCUEngine::evaluateMove_damage_damagingMoveBase() {
  const Move& cMove = getMV().getBase();
  // moves that are not physical or special attacks skip all damage computation
  // stages:
  if (cMove.damageType_ == ATK_PHYSICAL || cMove.damageType_ == ATK_SPECIAL) {
    return;
  }

  gotoStackStage(StageType::PREDAMAGE);
}


void NeoPkCUEngine::evaluateMove_damage_evaluateCritChance() {
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
      critEnv.setCrit(getICTeam());
    } else {  // crits 100% of the time
      getBase().setCrit(getICTeam());
    }
  } else {
    // crit 0% of the time (continues normally)
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
    gotoStackStage(StageType::POSTTURN);
    return;
  }
}


void NeoPkCUEngine::evaluateMove_evaluateSecondaryHitChance() {
  assert(getBase().hasHit(getICTeam()));

  const Move& cMove = getMV().getBase();
  FixType& probabilityToSecondary = getDamageComponent().tProbability;
  probabilityToSecondary = cMove.getSecondaryAccuracy();

  int result = 0;
  CALLPLUGIN(
      result,
      PLUGIN_ON_MODIFYSECONDARYPROBABILITY,
      onModifyProbability_rawType,
      *this,
      getMV(),
      getPKV(),
      getTPKV(),
      probabilityToSecondary);

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
    }
    // secondary occurs 100% of the time:
    getBase().setSecondary(getICTeam());
  } else {  // secondary occurs 0% of the time:
    gotoStackStage(StageType::POSTTURN);
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
  std::vector<Actor>& targets = frame.targets[frame.moveOrder[frame.iActor]];

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
    if (nextActor < actions_.size()) {
      frame.iActor = nextActor;
      frame.iTarget = 0;
      gotoStackStage(StageType::PRETURN);
    } else {
      // no more actors or targets. Next stage
      frame.iActor = 0;
      frame.iTarget = 0;
      advanceStackStage(frame.iStack);
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


void NeoPkCUEngine::evaluateMove_switch_onSwitchOut() {
  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_SWITCHOUT, onSwitch_rawType, *this, getPKV());
}

void NeoPkCUEngine::evaluateMove_switch_onSwitchIn() {
  getTV().swapPokemon(getCAction().iFriendly());
  getBase().setSwitched(getICTeam());

  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_SWITCHIN, onSwitch_rawType, *this, getPKV());

  gotoStackStage(StageType::POSTTURN);
}
