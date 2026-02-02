#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_sereneGrace(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToSecondary) {
  if (!cPKV.nv().abilityExists() ||
      (&(cPKV.nv().getAbility()) != sereneGrace_t)) {
    return 0;
  }

  uint8_t dType = mV.getBase().getDamageType();
  // must have used a physical or special attack move
  if (!((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL))) { return 0; }

  // multiply secondary probability by 2
  probabilityToSecondary = std::min(probabilityToSecondary * 2, FixType(1));

  return 1;
};

void register_ability_serene_grace(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "serene grace", PLUGIN_ON_MODIFYSECONDARYPROBABILITY, ability_sereneGrace, -1, current_team));
}

} // namespace gen4
