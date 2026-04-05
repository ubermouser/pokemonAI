#include "gen4_scripts_internal.h"

namespace gen4 {

int move_haze(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move* tMove = &mV.getBase();
  if (tMove != haze_t) {
    return 0;
  }

  // Helper lambda to reset stats
  auto reset_stats = [](PokemonVolatile& pkv) {
      if (pkv.isAlive()) {
          pkv.setBoost(FV_ATTACK, 0);
          pkv.setBoost(FV_SPATTACK, 0);
          pkv.setBoost(FV_DEFENSE, 0);
          pkv.setBoost(FV_SPDEFENSE, 0);
          pkv.setBoost(FV_SPEED, 0);
          pkv.setBoost(FV_ACCURACY, 0);
          pkv.setBoost(FV_EVASION, 0);
          pkv.setBoost(FV_CRITICALHIT, 0);
      }
  };

  // Reset stats for current team's active pokemon
  TeamVolatile cTV = cu.getTV();
  PokemonVolatile currentActive = cTV.getPKV();
  reset_stats(currentActive);

  // Reset stats for target team's active pokemon
  TeamVolatile tTV = cu.getTTV();
  PokemonVolatile targetActive = tTV.getPKV();
  reset_stats(targetActive);

  return 1;
};

void register_move_haze(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "haze", move_haze, 0, current_team));
}

} // namespace gen4
