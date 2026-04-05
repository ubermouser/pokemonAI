#include "gen4_scripts_internal.h"

namespace gen4 {


int move_trap_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move& cMove = mV.getBase();
  if ((&cMove != block_t) && (&cMove != meanLook_t) &&
      (&cMove != spiderWeb_t)) {
    return 0;
  }

  // Set fully trapped status
  tPKV.status().cTeammate.trap = 7;

  return 1;
}

int engine_checkTrapped(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile fPKV,
    const Action& action,
    ValidSwapSet& switchAllowed) {
  // If fully trapped (trap == 7), prevent switching
  if (cPKV.status().cTeammate.trap == 7) {
    switchAllowed[VALID_SWAP_SCRIPT] = false;
    return 1;
  }
  return 0;
}

int engine_clearTrap(PkCUEngine& cu, PokemonVolatile cPKV) {
  // cPKV is the pokemon switching in (or just switched in).

  // 1. Clear trap on self to ensure new arrivals aren't trapped
  if (cPKV.status().cTeammate.trap == 7) {
    cPKV.status().cTeammate.trap = 0;
  }

  // 2. Clear trap on opponent because the trapper (previous pokemon) has left.
  // Note: We assume that if the opponent is fully trapped (7), it was by us.
  PokemonVolatile tPKV = cu.getTPKV();

  if (tPKV.isAlive() && tPKV.status().cTeammate.trap == 7) {
    tPKV.status().cTeammate.trap = 0;
  }

  return 1;
}

void register_move_trap(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnEvaluateMove(move, "block", move_trap_set, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "mean look", move_trap_set, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "spider web", move_trap_set, 0, current_team));
  extensions.push_back(pluginOnTestSwitch(engine, "trapped check", engine_checkTrapped, 0, all_teams));
  extensions.push_back(pluginOnSwitchIn(engine, "clear trap on switch", engine_clearTrap, 0, all_teams));
  // clang-format on
}

} // namespace gen4
