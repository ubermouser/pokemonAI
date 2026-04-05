#include "gen4_scripts_internal.h"

namespace gen4 {

int move_facade_modPower(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& modifier) {
  if (&mV.getBase() != facade_t) { return 0; }

  uint32_t status = cPKV.getStatusAilment();
  if (status == AIL_NV_PARALYSIS || status == AIL_NV_POISON ||
      status == AIL_NV_POISON_TOXIC) {
    modifier *= 2.0;
  } else if (status == AIL_NV_BURN) {
    modifier *= 4.0;  // Double power (2x) and compensate for Burn halving
                      // attack (2x) = 4x
  }

  return 1;
};

void register_move_facade(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnModifyBasePower(move, "facade", move_facade_modPower, 0, current_team));
  // clang-format on
}

} // namespace gen4
