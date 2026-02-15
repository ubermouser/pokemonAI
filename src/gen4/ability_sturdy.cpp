#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_sturdy(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToHit) {
  const Move* cMove = &mV.getBase();
  if (cMove == fissure_t || cMove == guillotine_t || cMove == hornDrill_t ||
      cMove == sheerCold_t) {
    probabilityToHit = FixType(0.0);
    return 2;
  }

  return 0;
};

void register_ability_sturdy(
    const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(
      ability, "sturdy", PLUGIN_ON_MODIFYHITPROBABILITY, ability_sturdy, -10,
      other_team));
}

} // namespace gen4
