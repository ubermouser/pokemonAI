#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_innerFocus(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (!cPKV.nv().abilityExists() || (&cPKV.nv().getAbility() != innerFocus_t)) {
    return 0;
  }
  cPKV.status().cTeammate.flinch = 0;
  return 1;
}

void register_ability_inner_focus(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(plugin(ability, "inner focus", PLUGIN_ON_BEGINNINGOFTURN, ability_innerFocus, -3, current_team));
  // clang-format on
}

} // namespace gen4
