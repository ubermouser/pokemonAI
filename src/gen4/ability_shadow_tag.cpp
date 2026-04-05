#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_shadowTag(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile fPKV,
    const Action& action,
    ValidSwapSet& switchAllowed) {
  if (cPKV.nv().abilityExists() && (&cPKV.nv().getAbility() == shadowTag_t)) {
    return 0;
  }

  // Shadow Tag does not affect fainted pokemon
  if (!cPKV.isAlive()) { return 0; }

  switchAllowed[VALID_SWAP_SCRIPT] = false;
  return 1;
};

void register_ability_shadow_tag(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnTestSwitch(ability, "shadow tag", ability_shadowTag, 0, other_team));
}

} // namespace gen4
