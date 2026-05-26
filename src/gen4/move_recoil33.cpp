#include "gen4_scripts_internal.h"

namespace gen4 {


int move_recoil33(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().flagsFor((TEAM)cu.getICTeam()).isHit()) { return 0; }

  const Move* cMove = &cu.getMV(actor).getBase();
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
  extensions.push_back(pluginOnEndOfMove(move, "brave bird", move_recoil33, -1, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "double-edge", move_recoil33, -1, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "flare blitz", move_recoil33, -1, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "wood hammer", move_recoil33, -1, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "volt tackle", move_recoil33, -1, current_team));
}

} // namespace gen4
