#include "gen4_scripts_internal.h"

namespace gen4 {

int move_endeavor(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != endeavor_t) { return 0; }

  // Check Type Immunity (Normal vs Ghost)
  // Endeavor is Normal type.
  const PokemonBase& tPKB = tPKV.getBase();
  if ((&tPKB.getType(0) == ghost_t) ||
      (&tPKB.getType(1) == ghost_t)) {
      // Immune
      return 1;
  }

  // Endeavor fails if the user's HP is greater than or equal to the target's HP.
  if (cPKV.getHP() >= tPKV.getHP()) {
      return 1;
  }

  // The target's HP is set to equal the user's HP.
  int32_t damage = (int32_t)tPKV.getHP() - (int32_t)cPKV.getHP();
  tPKV.modHP(-damage);

  return 1;
}

void register_move_endeavor(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "endeavor", PLUGIN_ON_EVALUATEMOVE, move_endeavor, 0, current_team));
}

} // namespace gen4
