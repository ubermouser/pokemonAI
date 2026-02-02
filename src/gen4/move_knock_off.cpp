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

  // Check if move hit and dealt damage
  // If damage is 0 (miss, immunity, etc.), Knock Off fails to remove item.
  if (cu.getDamageComponent().damage == 0) { return 1; }

  // User must be alive (e.g. didn't faint from recoil)
  if (!cPKV.isAlive()) { return 1; }

  // If the target has a substitute, the item is not knocked off.
  if (tPKV.status().cTeammate.substitute > 0) { return 1; }

  // Target must be alive to lose item
  if (!tPKV.isAlive()) { return 1; }

  if (tPKV.nv().abilityExists()) {
    const auto& ability = tPKV.nv().getAbility();
    if (&ability == stickyHold_t) { return 1; }
  }

  if (tPKV.hasItem()) {
    const std::string& itemName = tPKV.getItem().getName();
    std::string itemNameLower = itemName;
    std::transform(itemNameLower.begin(), itemNameLower.end(), itemNameLower.begin(), ::tolower);

    const std::string& speciesName = tPKV.getBase().getName();
    std::string speciesNameLower = speciesName;
    std::transform(speciesNameLower.begin(), speciesNameLower.end(), speciesNameLower.begin(), ::tolower);

    // Arceus holding a Plate
    if (speciesNameLower.find("arceus") != std::string::npos) {
        if (itemNameLower.find("plate") != std::string::npos) {
            return 1;
        }
    }

    // Giratina holding Griseous Orb
    if (speciesNameLower.find("giratina") != std::string::npos) {
        if (itemNameLower == "griseous orb") {
            return 1;
        }
    }

    tPKV.setNoItem();
  }

  return 1;
};

void register_move_knock_off(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "knock off", PLUGIN_ON_ENDOFMOVE, move_knockOff, 0, current_team));
}

} // namespace gen4
