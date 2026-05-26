#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_naturalCure(
    PkCUEngine& cu,
    const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (!cPKV.nv().abilityExists() ||
      (&(cPKV.nv().getAbility()) != naturalCure_t)) {
    return 0;
  }

  // clear status ailment on switchout
  cPKV.clearStatusAilment();

  return 0;
};

void register_ability_natural_cure(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnSwitchOut(ability, "natural cure", ability_naturalCure, 0, current_team));
}

} // namespace gen4
