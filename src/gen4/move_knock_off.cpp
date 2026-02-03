#include "gen4_scripts_internal.h"
#include <algorithm>
#include <string>

namespace gen4 {

int move_knockOff(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != knockOff_t) { return 0; }

  // Check if move hit
  // If damage is 0 (miss, immunity, etc.), Knock Off fails to remove item.
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 1; }

  // If the target has a substitute, the item is not knocked off.
  if (tPKV.status().cTeammate.substitute > 0) { return 1; }

  // Target must be alive to lose item
  if (!tPKV.isAlive()) { return 1; }

  if (tPKV.nv().abilityExists()) {
    const auto& ability = tPKV.nv().getAbility();
    if (&ability == stickyHold_t) { return 1; }
  }

  if (tPKV.hasItem()) {
    tPKV.setNoItem();
  }

  return 1;
};

void register_move_knock_off(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "knock off", PLUGIN_ON_ENDOFMOVE, move_knockOff, 0, current_team));
}

} // namespace gen4
