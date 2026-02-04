#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_sturdy(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToHit) {

  // Sturdy is an ability on the defender (tPKV)
  if (!tPKV.nv().abilityExists() || (&(tPKV.nv().getAbility()) != sturdy_t)) {
    return 0;
  }

  const Move* tMove = &mV.getBase();
  bool isOHKO = (tMove == fissure_t) || (tMove == sheer_cold_t) || (tMove == guillotine_t) || (tMove == horn_drill_t);

  if (isOHKO) {
    probabilityToHit = FixType(0.0f);
    return 1;
  }

  return 0;
}

void register_ability_sturdy(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // Priority 1 to run AFTER the move's own accuracy calculation (which is typically priority 0)
  extensions.push_back(plugin(ability, "sturdy", PLUGIN_ON_MODIFYHITPROBABILITY, ability_sturdy, 1, other_team));
}

} // namespace gen4
