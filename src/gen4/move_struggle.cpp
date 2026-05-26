#include "gen4_scripts_internal.h"

namespace gen4 {

int move_struggle_recoil25(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().flagsFor((TEAM)cu.getICTeam()).isHit()) { return 0; }

  const Move* cMove = &cu.getMV(actor).getBase();

  if (cMove != struggle_t) { return 0; }

  // subtract hitpoints:
  cPKV.modPercentHP(-0.25);

  return cPKV.isAlive() ? 1 : 2;
};

void register_move_struggle(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnEndOfMove(engine, "struggle damage effect", move_struggle_recoil25, 0, all_teams));
  // always hits effect is in move_alwaysHits.cpp

  // clang-format on
}

} // namespace gen4
