#include "gen4_scripts_internal.h"

namespace gen4 {

int move_refresh(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != refresh_t) { return 0; }

  uint32_t status = cPKV.getStatusAilment();
  if (status == AIL_NV_BURN || status == AIL_NV_POISON ||
      status == AIL_NV_POISON_TOXIC || status == AIL_NV_PARALYSIS) {
    cPKV.clearStatusAilment();
    // Reset toxic counter if it was toxic poison
    if (status == AIL_NV_POISON_TOXIC) {
      cPKV.status().cTeammate.toxicPoison_tier = 0;
    }
    return 1;
  }

  return 0;
}

void register_move_refresh(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(
      pluginOnEvaluateMove(move, "refresh", move_refresh, 0, current_team));
}

} // namespace gen4
