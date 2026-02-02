#include "gen1_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen1 {

int engine_move_struggle(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const Move* cMove = &mV.getBase();

  if (cMove != struggle_t) { return 0; }

  // subtract hitpoints:
  cPKV.modPercentHP(-0.25);

  return cPKV.isAlive() ? 1 : 2;
};

void register_move_struggle(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(engine, "struggle damage effect", PLUGIN_ON_ENDOFMOVE, engine_move_struggle, 0, all_teams));
  // always hits effect is in move_alwaysHits.cpp
}

} // namespace gen1
