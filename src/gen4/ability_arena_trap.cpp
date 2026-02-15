#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_arenaTrap(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile fPKV,
    ConstPokemonVolatile tPKV,
    const Action& action,
    ValidSwapSet& switchAllowed) {
  return trapped_by_ability_common(cPKV, tPKV, true, false, arenaTrap_t, switchAllowed);
}

void register_ability_arena_trap(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "arena trap", PLUGIN_ON_TESTSWITCH, ability_arenaTrap, 0, other_team));
}

} // namespace gen4
