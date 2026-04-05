#include "gen4_scripts_internal.h"

namespace gen4 {

// Needed to show that this ability is registered (through Trick)
int ability_doNothing(PkCUEngine& cu, PokemonVolatile cPKV) { return 0; };

void register_ability_sticky_hold(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnSwitchOut(ability, "sticky hold", ability_doNothing, 99, current_team));
  // clang-format on
}

} // namespace gen4
