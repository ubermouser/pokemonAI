#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {


int move_recoil33(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const Move* cMove = &mV.getBase();
  if ((cMove != doubleEdge_t) && (cMove != woodHammer_t) &&
      (cMove != flareBlitz_t) && (cMove != braveBird_t) &&
      (cMove != voltTackle_t)) {
    return 0;
  }

  // subtract hitpoints:
  cPKV.modHP((int32_t)cu.getDamageComponent().damage / -3);

  return cPKV.isAlive() ? 1 : 2;
};


void register_move_recoil33(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "brave bird", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, current_team));
  extensions.push_back(plugin(move, "double-edge", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, current_team));
  extensions.push_back(plugin(move, "flare blitz", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, current_team));
  extensions.push_back(plugin(move, "wood hammer", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, current_team));
  extensions.push_back(plugin(move, "volt tackle", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, current_team));
}

} // namespace gen4
