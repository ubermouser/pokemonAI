#ifndef INNER_FOCUS_CPP
#define INNER_FOCUS_CPP

#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int ability_innerFocus(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (!cPKV.nv().abilityExists() || (&cPKV.nv().getAbility() != innerFocus_t)) {
    return 0;
  }
  cPKV.status().cTeammate.flinch = 0;
  return 1;
}

void register_ability_inner_focus(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "inner focus", PLUGIN_ON_BEGINNINGOFTURN, ability_innerFocus, 0, current_team));
}

} // namespace gen4

#endif
