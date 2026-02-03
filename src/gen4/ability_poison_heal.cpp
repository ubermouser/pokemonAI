#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_poison_heal_init(PkCUEngine&, PokemonVolatile) {
  return 0;
}

void register_ability_poison_heal(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "poison heal", PLUGIN_ON_INIT, ability_poison_heal_init, 0, current_team));
}

} // namespace gen4
