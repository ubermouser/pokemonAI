#include "gen4_scripts_internal.h"

namespace gen4 {

int move_lifeLeech50(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().flagsFor((TEAM)cu.getICTeam()).isHit()) { return 0; }

  const Move* cMove = &cu.getMV(actor).getBase();
  if ((cMove != absorb_t) && (cMove != leechLife_t) && (cMove != gigaDrain_t) &&
      (cMove != megaDrain_t) && (cMove != drainPunch_t)) {
    return 0;
  }

  // add to hitpoints:
  cPKV.modHP(cu.getDamageComponent().damage / 2);

  return 1;
};

void register_move_lifeLeech50(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfMove(move, "absorb", move_lifeLeech50, 0, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "drain punch", move_lifeLeech50, 0, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "giga drain", move_lifeLeech50, 0, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "leech life", move_lifeLeech50, 0, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "mega drain", move_lifeLeech50, 0, current_team));
}

} // namespace gen4
