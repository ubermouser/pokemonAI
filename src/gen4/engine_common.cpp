#include "gen4_scripts_internal.h"

namespace gen4 {

int engine_modifyAttackPower_burn(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& modifier) {
  modifier *= ((cPKV.getStatusAilment() == AIL_NV_BURN) &&
               (mV.getBase().getDamageType() == ATK_PHYSICAL))
                  ? 0.5
                  : 1.0;

  return 1;
};

int engine_onModifySpeed_paralyze(
    PkCUEngine&, PokemonVolatile cPKV, uint32_t& speed) {
  // divide by 4 if pokemon is paralyzed
  speed /= (cPKV.getStatusAilment() == AIL_NV_PARALYSIS) ? 4 : 1;

  return 1;
};

int engine_endRoundDamageEffect(PkCUEngine& cu, PokemonVolatile cPKV) {
  // nonvolatile:
  uint32_t condition = cPKV.getStatusAilment();
  if (condition == AIL_NV_POISON || condition == AIL_NV_BURN) {
    // reduce HP of pokemon by (1/8) or .125
    cPKV.modPercentHP(-0.125);
  } else if (condition == AIL_NV_POISON_TOXIC) {
    uint32_t toxicTier = cPKV.status().cTeammate.toxicPoison_tier;

    // increment toxic tier, more added damage per round
    if (toxicTier < 15) { cPKV.status().cTeammate.toxicPoison_tier++; }

    cPKV.modPercentHP(-0.0625 * (fpType)(toxicTier + 1));
  }

  // volatile:
  // flinch only lasts for the current round. Only a pokemon moving first can
  // flinch the other pokemon
  cPKV.status().cTeammate.flinch = 0;

  return (cPKV.isAlive() ? 1 : 2);
};

int engine_beginTurnNonvolatileEffect(PkCUEngine& cu, PokemonVolatile cPKV) {
  // Does this pokemon have a non-volatile condition?
  uint32_t cStatus = cPKV.getStatusAilment();
  switch (cStatus) {
  case AIL_NV_FREEZE: {
    // 100% chance for certain fire moves to thaw the user:
    const Move& cMove = cu.getMV().getBase();
    if (&cMove == flameWheel_t || &cMove == flareBlitz_t) {
      cPKV.clearStatusAilment();
      break;
    }

    // generate a new environment on the result array:
    std::array<size_t, 2> iREnv;
    cu.duplicateState(iREnv, FixType(0.8f));

    // 80% chance for frozen status effect to prevent user from moving:
    {
      // modify the status environment:
      EnvironmentPossible statEnv = cu.getStack().at(iREnv[1]);
      statEnv.flagsFor(cu.getCActor()).setBlocked();
    }
    // 20% chance for pokemon to thaw:
    {
      cu.getPKV(iREnv[0]).clearStatusAilment();
    }
    break;
  }
  case AIL_NV_SLEEP_4T:
  case AIL_NV_SLEEP_3T:
  case AIL_NV_SLEEP_2T:
  case AIL_NV_SLEEP_1T: {
    static const std::array<FixType, 4> sleepStatusProb = {
        {FixType(0.5), FixType(1.0 / 3.0), FixType(0.25), FixType(0.0)}};
    // decrement sleep counter (no effect until next turn)
    cPKV.setStatusAilment(cPKV.getStatusAilment() - 1);

    uint32_t iSleepProb =
        std::min((uint32_t)4, (uint32_t)(cStatus - AIL_NV_SLEEP_0T)) - 1;
    std::array<size_t, 2> iREnv = {cu.getIBase(), SIZE_MAX};
    if (iSleepProb != 3) {
      // generate a new environment on the result array:
      cu.duplicateState(iREnv, sleepStatusProb[iSleepProb]);

      // variable % chance for the pokemon to move this turn:
      cu.getPKV(iREnv[1]).clearStatusAilment();
    }
    // pokemon has a chance to move this turn:
    cu.getStack().at(iREnv[0]).flagsFor(cu.getCActor()).setBlocked();
    break;
  }
  case AIL_NV_REST_3T:
  case AIL_NV_REST_2T: {
    // pokemon is sleeping for a fixed number of turns
    cPKV.setStatusAilment(cStatus - 1);
    cu.getBase().flagsFor(cu.getCActor()).setBlocked();
    break;
  }
  case AIL_NV_REST_1T: {
    // pokemon wakes up from rest
    cPKV.clearStatusAilment();
    break;
  }
  case AIL_NV_PARALYSIS: {
    // generate a new environment on the result array:
    std::array<size_t, 2> iREnv;
    cu.duplicateState(iREnv, FixType(0.25));
    // 25% chance to be paralyzed and not move
    cu.getStack().at(iREnv[1]).flagsFor(cu.getCActor()).setBlocked();
    break;
  }
  case AIL_NV_NONE:
  default:
    // don't do anything if no status condition
    break;
  }  // endOf nonVolatile switch

  return 1;
}  // endOf begin turn nonvolatile effect

int engine_beginTurnVolatileEffect(PkCUEngine& cu, PokemonVolatile cPKV) {
  // Does this pokemon have a volatile condition?
  if (cPKV.status().cTeammate.flinch > 0) {
    // set user blocked 100% of the time
    cu.getBase().flagsFor(cu.getCActor()).setBlocked();
  }
  if (cPKV.status().cTeammate.infatuate > 0) {
    // 50% chance to move:
    std::array<size_t, 2> iREnv;
    cu.duplicateState(iREnv, FixType(0.5));

    // 50% chance to not move:
    {
      cu.getStack().at(iREnv[1]).flagsFor(cu.getCActor()).setBlocked();
    }
  }
  if (cPKV.status().cTeammate.confused > 0) {
    uint32_t iConfused = cPKV.status().cTeammate.confused;
    if (iConfused != AIL_V_CONFUSED_0T) {
      // 50% chance to move:
      std::array<size_t, 2> iREnv;
      cu.duplicateState(iREnv, FixType(0.5));

      PokemonVolatile cConfusedPKV = cu.getPKV(iREnv[1]);

      // 50% chance to not move:
      {
        cu.getStack().at(iREnv[1]).flagsFor(cu.getCActor()).setBlocked();
        cConfusedPKV.status().cTeammate.confused--;
        // TODO: actual damage calculation
        cConfusedPKV.modHP(-40);
      }
      // if pokemon did not kill its self with hurt confusion:
      if (cConfusedPKV.isAlive()) {
        uint32_t numTotalEnv =
            std::min((unsigned)4, iConfused - AIL_V_CONFUSED_0T);
        uint32_t numTerminalEnv =
            ((iConfused - AIL_V_CONFUSED_0T) >= 5) ? 0 : 1;
        FixType terminalProbability = FixType(numTerminalEnv, numTotalEnv);

        std::array<size_t, 2> iTEnv;

        if ((numTerminalEnv > 0) && (numTotalEnv > numTerminalEnv)) {
          cu.duplicateState(iTEnv, terminalProbability, iREnv[1]);

          // variable % chance for this env to be the last environment confused:
          cConfusedPKV.status().cTeammate.confused = 0;
        }
      }
    } else /* equals AIL_V_CONFUSED_0T */
    {
      // pokemon breaks out of confusion this round
      cPKV.status().cTeammate.confused = 0;
    }
  }  // end of confused

  return 1;
}  // endOf begin turn volatile effect

int engine_secondaryBoostEffect(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move& cMove = mV.getBase();

  // apply buffs to the current pokemon, and debuffs to the other pokemon:
  for (size_t iBuff = 0; iBuff != 9; ++iBuff) {
    cPKV.modBoost(iBuff, cMove.getSelfBuff(iBuff));
  }

  // all other effects modify the target pokemon, and we don't want to modify a
  // dead one
  //  (this will stop all other plugins from running as well)
  if (!tPKV.isAlive()) { return 2; }

  for (size_t iBuff = 0; iBuff != 9; ++iBuff) {
    tPKV.modBoost(iBuff, -1 * cMove.getTargetDebuff(iBuff));
  }

  return 1;
}  // endOf apply buffs / debuffs

int engine_secondaryNonvolatileEffect(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move& cMove = mV.getBase();

  if (tPKV.getStatusAilment() != AIL_NV_NONE) { return 0; }

  // apply status conditions to the other pokemon:
  switch (cMove.getTargetAilment()) {
  case AIL_NV_SLEEP:
  case AIL_NV_POISON_TOXIC:
  case AIL_NV_FREEZE:
  case AIL_NV_BURN:
  case AIL_NV_PARALYSIS:
  case AIL_NV_POISON:
  default:
    // reset toxic tier:
    tPKV.status().cTeammate.toxicPoison_tier = 0;
    // apply generic status condition
    tPKV.setStatusAilment(cMove.getTargetAilment());
    // implicitly push back bEnv status condition (already on array)
    break;
  case AIL_NV_NONE:
    break;  // do not apply a status condition, or push anything back
  }  // end of targetAilment switch
  return 1;
}  // endOf apply buffs / debuffs

int engine_secondaryVolatileEffect(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // apply volatile status conditions to the other pokemon:
  switch (mV.getBase().getTargetVolatileAilment()) {
  case AIL_V_CONFUSED:
    // confused for (at most) 5 turns, and (at least) 2 turns:
    tPKV.status().cTeammate.confused = AIL_V_CONFUSED_5T;
    // implicitly push back bEnv
    break;
  case AIL_V_FLINCH:
    tPKV.status().cTeammate.flinch = 1;
    break;
  case AIL_V_INFATUATED:
    tPKV.status().cTeammate.infatuate = 1;
    break;
  default:
  case AIL_V_NONE:
    break;  // do not apply a status condition, or push anything back
  }  // endOf targetVolatileAilment switch
  return 1;
}  // endOf apply buffs / debuffs

int engine_decrementPP(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // don't decrement PP if this move is struggle_t or the move did not hit
  if (!cu.getBase().flagsFor(cu.getCActor()).isHit() || (&mV.getBase() == struggle_t)) {
    return 0;
  }

  mV.modPP(-1);

  return 1;
};

int engine_updateLastAction(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (!cPKV.isAlive()) { return 0; }

  const auto& lastAction = cu.getCAction();
  bool switched = cu.getBase().flagsFor(cu.getCActor()).isSwitched();
  if (lastAction.isMove() && !switched) {
    cPKV.status().cTeammate.iLastAction = lastAction.iMove() + 1;
    return 1;
  } else {
    cPKV.status().cTeammate.iLastAction = 0;
    return 0;
  }
};

int engine_thawOnFireHit(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& damage) {
  if (damage > 0 && &mV.getBase().getType() == fire_t &&
      tPKV.getStatusAilment() == AIL_NV_FREEZE) {
    tPKV.clearStatusAilment();
  }
  return 1;
}

void register_engine_common(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnEndOfTurn(engine, "engine record last action", engine_updateLastAction, 1, all_teams));
  extensions.push_back(pluginOnEndOfMove(engine, "pp decrement", engine_decrementPP, 0, all_teams));
  extensions.push_back(pluginOnModifySpeed(engine, "nonvolatile speed change", engine_onModifySpeed_paralyze, -1, all_teams));
  extensions.push_back(pluginOnBeginningOfTurn(engine, "nonvolatile beginning-of-round damage", engine_beginTurnNonvolatileEffect, -2, all_teams));
  extensions.push_back(pluginOnBeginningOfTurn(engine, "volatile beginning-of-round damage", engine_beginTurnVolatileEffect, -1, all_teams));
  extensions.push_back(pluginOnSecondaryEffect(engine, "secondary effect boosts", engine_secondaryBoostEffect, -3, all_teams));
  extensions.push_back(pluginOnSecondaryEffect(engine, "secondary effect nonvolatile", engine_secondaryNonvolatileEffect, -2, all_teams));
  extensions.push_back(pluginOnSecondaryEffect(engine, "secondary effect volatile", engine_secondaryVolatileEffect, -1, all_teams));
  extensions.push_back(pluginOnEndOfRound(engine, "nonvolatile end-of-round damage", engine_endRoundDamageEffect, 0, all_teams));
  extensions.push_back(pluginOnModifyAttackPower(engine, "damage mod burn", engine_modifyAttackPower_burn, 0, all_teams));
  extensions.push_back(pluginOnCalculateDamage(engine, "thaw on fire hit", engine_thawOnFireHit, 0, all_teams));
  // clang-format on
}

} // namespace gen4
