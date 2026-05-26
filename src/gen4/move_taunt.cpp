#include "gen4_scripts_internal.h"

namespace gen4 {

int move_taunt_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != taunt_t) { return 0; }

  // Fails if the target is already taunted.
  if (tPKV.status().cTeammate.taunt_duration > 0) { return 1; }

  tPKV.status().cTeammate.taunt_duration = 5;

  return 1;
}

int move_taunt_test(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  if (cPKV.status().cTeammate.taunt_duration == 0) { return 0; }

  // if taunted, cannot use status moves
  if (mV.getBase().getDamageType() == ATK_NODMG) {
    moveAllowed[VALID_MOVE_SCRIPT] = false;
  }

  return 1;
}

int move_taunt_preempt(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  auto& teamStatus = cPKV.status().cTeammate;
  if (teamStatus.taunt_duration == 0) { return 0; }

  uint32_t duration = teamStatus.taunt_duration;

  if (duration == 2 || duration == 1) {
    std::array<size_t, 2> iREnv;
    cu.duplicateState(iREnv, FixType(1, duration + 1)); // probability to end early: 1 / (duration + 1)

    // Case 1: Taunt continues
    {
      auto& newStatus = cu.getPKV(iREnv[0]).status().cTeammate;
      newStatus.taunt_duration = duration - 1;

      // Block status move if chosen
      if (cu.getMV(iREnv[0]).getBase().getDamageType() == ATK_NODMG) {
        cu.getBase(iREnv[0]).flagsFor(cu.getCActor(iREnv[0])).setBlocked();
      }
    }
    // Case 2: Taunt ends
    {
      auto& newStatus = cu.getPKV(iREnv[1]).status().cTeammate;
      newStatus.taunt_duration = 0;
    }
  } else {
    // duration > 2 (i.e. 5, 4, 3)
    teamStatus.taunt_duration = duration - 1;
    // Block status move if chosen
    if (cu.getMV().getBase().getDamageType() == ATK_NODMG) {
      cu.getBase().flagsFor(cu.getCActor()).setBlocked();
    }
  }

  return 1;
}

void register_move_taunt(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnEvaluateMove(move, "taunt", move_taunt_set, 0, current_team));
  extensions.push_back(pluginOnTestMove(engine, "taunt_test", move_taunt_test, 0, all_teams));
  extensions.push_back(pluginOnBeginningOfTurn(engine, "taunt_preempt", move_taunt_preempt, -1, all_teams));
  // clang-format on
}

} // namespace gen4
