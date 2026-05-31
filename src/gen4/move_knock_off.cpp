#include "gen4_scripts_internal.h"
#include <algorithm>
#include <string>

namespace gen4 {

int move_knockOff(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != knockOff_t) { return 0; }

  // Check if move hit
  // If damage is 0 (miss, immunity, etc.), Knock Off fails to remove item.
  if (!cu.getBase().flagsFor((TEAM)cu.getICTeam()).isHit()) { return 1; }

  // If the target has a substitute, the item is not knocked off.
  if (tPKV.status().substitute > 0) { return 1; }

  // Target must be alive to lose item
  if (!tPKV.isAlive()) { return 1; }

  if (&tPKV.nv().getAbility() == stickyHold_t) { return 1; }

  if (tPKV.hasItem()) {
    tPKV.setNoItem();
  }

  return 1;
};

void register_move_knock_off(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfMove(move, "knock off", move_knockOff, 0, current_team));
}

} // namespace gen4
