#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_magnetPull(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile fPKV,
    ConstPokemonVolatile tPKV,
    const Action& action,
    ValidSwapSet& switchAllowed) {
  return trapped_by_ability_common(cPKV, tPKV, false, true, magnetPull_t, switchAllowed);
}

void register_ability_magnet_pull(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "magnet pull", PLUGIN_ON_TESTSWITCH, ability_magnetPull, 0, other_team));
}

} // namespace gen4
