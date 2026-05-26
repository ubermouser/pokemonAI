#include "gen4_scripts_internal.h"

namespace gen4 {

int move_haze(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  PokemonVolatile tPKV = cu.getPKV(target);
  const Move* tMove = &cu.getMV(actor).getBase();
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

  // Reset stats for current and target team's active pokemon
  reset_stats(cPKV);
  reset_stats(tPKV);

  return 1;
};

void register_move_haze(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "haze", move_haze, 0, current_team));
}

} // namespace gen4
