#include "gen4_scripts_internal.h"

namespace gen4 {

int move_belly_drum(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != bellyDrum_t) { return 0; }

  // Fails if HP is not enough (<= 50%)
  if (cPKV.getHP() <= cPKV.nv().getMaxHP() / 2) { return 1; }

  // Fails if Attack is already at max (+6)
  if (cPKV.getBoost(FV_ATTACK) == 6) { return 1; }

  cPKV.modHP(-(int32_t)(cPKV.nv().getMaxHP() / 2));
  cPKV.setBoost(FV_ATTACK, 6);

  return 1;
};

int move_belly_drum_secondary_probability(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    FixType& probability) {
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != bellyDrum_t) { return 0; }

  probability = FixType(0);
  return 1;
}

void register_move_belly_drum(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "belly drum", move_belly_drum, 0, current_team));
  extensions.push_back(pluginOnModifySecondaryProbability(move, "belly drum", move_belly_drum_secondary_probability, 0, current_team));
}

} // namespace gen4
