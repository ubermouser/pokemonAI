#include "gen4_scripts_internal.h"

namespace gen4 {

int move_lifeLeech50(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const Move* cMove = &mV.getBase();
  if ((cMove != absorb_t) && (cMove != leechLife_t) && (cMove != gigaDrain_t) &&
      (cMove != megaDrain_t) && (cMove != drainPunch_t)) {
    return 0;
  }

  // add to hitpoints:
  cPKV.modHP(cu.getDamageComponent().damage / 2);

  return 1;
};

void register_move_lifeLeech50(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "absorb", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, current_team));
  extensions.push_back(plugin(move, "drain punch", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, current_team));
  extensions.push_back(plugin(move, "giga drain", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, current_team));
  extensions.push_back(plugin(move, "leech life", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, current_team));
  extensions.push_back(plugin(move, "mega drain", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, current_team));
}

} // namespace gen4
