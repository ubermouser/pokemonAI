#include "gen4_scripts_internal.h"

namespace gen4 {

int move_taunt_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != taunt_t) { return 0; }

  std::array<size_t, 3> iREnv;
  // equal probability for 3, 4, and 5 turns
  cu.triplicateState(iREnv, FixType(1.0f / 3.0f), FixType(1.0f / 3.0f));

  // case 1: 3 turns
  {
    PokemonVolatile tPKV = cu.getTPKV(iREnv[0]);
    tPKV.status().cTeammate.taunt_duration = 3;
  }
  // case 2: 4 turns
  {
    PokemonVolatile tPKV = cu.getTPKV(iREnv[1]);
    tPKV.status().cTeammate.taunt_duration = 4;
  }
  // case 3: 5 turns
  {
    PokemonVolatile tPKV = cu.getTPKV(iREnv[2]);
    tPKV.status().cTeammate.taunt_duration = 5;
  }

  return 1;
};

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

int move_taunt_preempt(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (cPKV.status().cTeammate.taunt_duration > 0) {
    if (cu.getMV().getBase().getDamageType() == ATK_NODMG) {
      cu.getBase().flagsFor(cu.getCActor()).setBlocked();
    }

    cPKV.status().cTeammate.taunt_duration--;
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
