#include "gen4_scripts_internal.h"

namespace gen4 {


int move_payback_modPower(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != payback_t) { return 0; }

  // if the enemy's move is NOT a damaging move:
  const Action& oAction = cu.getOAction();
  bool enemyMoveAction = (cu.getOAction().isMove());
  // if the enemy moves first:
  bool enemyMovedFirst =
      cu.getBase().flagsFor((TEAM)cu.getIOTeam()).isMovedFirst();

  if (!enemyMoveAction || !enemyMovedFirst) { return 1; }

  // greatly increase the power of the move:
  modifier *= 2.0;
  return 1;
}

void register_move_payback(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyRawDamage(move, "payback", move_payback_modPower, 0, current_team));
}

} // namespace gen4
