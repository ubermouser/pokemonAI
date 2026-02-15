#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_shadowTag(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile fPKV,
    ConstPokemonVolatile tPKV,
    const Action& action,
    ValidSwapSet& switchAllowed) {
  // Shadow Tag bypass: switcher has Shadow Tag
  if (cPKV.nv().abilityExists() && (&cPKV.nv().getAbility() == shadowTag_t)) {
    return 0;
  }
  return trapped_by_ability_common(cPKV, tPKV, false, false, shadowTag_t, switchAllowed);
}

void register_ability_shadow_tag(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "shadow tag", PLUGIN_ON_TESTSWITCH, ability_shadowTag, 0, other_team));
}

} // namespace gen4
