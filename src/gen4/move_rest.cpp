#include "gen4_scripts_internal.h"

namespace gen4 {

int move_rest(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != rest_t) { return 0; }
  // Rest fails if HP is full
  if (cPKV.getHP() == cPKV.nv().getMaxHP()) {
    return 0;
  }

  // Rest heals to full HP
  cPKV.setPercentHP(1.0);

  // Rest sets status to AIL_NV_REST
  cPKV.setStatusAilment(AIL_NV_REST);

  return 1;
}

void register_move_rest(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(
      pluginOnEvaluateMove(move, "rest", move_rest, 0, current_team));
}

} // namespace gen4
