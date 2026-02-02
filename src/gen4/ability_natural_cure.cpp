#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_naturalCure(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (!cPKV.nv().abilityExists() ||
      (&(cPKV.nv().getAbility()) != naturalCure_t)) {
    return 0;
  }

  // clear status ailment on switchout
  cPKV.clearStatusAilment();

  return 0;
};

void register_ability_natural_cure(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "natural cure", PLUGIN_ON_SWITCHOUT, ability_naturalCure, 0, current_team));
}

} // namespace gen4
