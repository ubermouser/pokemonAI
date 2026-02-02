#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {


int move_payback_modPower(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& modifier) {
  if (&mV.getBase() != payback_t) { return 0; }

  // if the enemy's move is NOT a damaging move:
  const Action& oAction = cu.getOAction();
  bool enemyMoveAction = (cu.getOAction().isMove());
  // if the enemy moves first:
  bool enemyMovedFirst = cu.getBase().hasMovedFirst(cu.getIOTeam());

  if (!enemyMoveAction || !enemyMovedFirst) { return 1; }

  // greatly increase the power of the move:
  modifier *= 2.0;
  return 1;
}

void register_move_payback(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "payback", PLUGIN_ON_MODIFYRAWDAMAGE, move_payback_modPower, 0, current_team));
}

} // namespace gen4
