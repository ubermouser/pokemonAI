#include "gen4_scripts_internal.h"

namespace gen4 {

int move_painSplit(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != painSplit_t) { return 0; }

  // calculate how much health total pokemon both have; average the two,
  // rounding down
  uint32_t newHP = (cPKV.getHP() + tPKV.getHP() + 1) / 2;

  cPKV.setHP(newHP);
  tPKV.setHP(newHP);

  return 1;
};

void register_move_pain_split(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "pain split", move_painSplit, 0, current_team));
}

} // namespace gen4
