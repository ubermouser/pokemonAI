#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_pressure(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().flagsFor((TEAM)cu.getICTeam()).isHit()) { return 0; }

  // Struggle is not affected by Pressure
  if (&mV.getBase() == struggle_t) { return 0; }

  mV.modPP(-1);

  return 1;
};

void register_ability_pressure(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfMove(ability, "pressure", ability_pressure, 0, other_team));
}

} // namespace gen4
