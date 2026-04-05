#include "gen4_scripts_internal.h"

namespace gen4 {

int move_tri_attack_secondary(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != triAttack_t) { return 0; }

  // Check if target already has status
  if (tPKV.getStatusAilment() != AIL_NV_NONE) {
    return 0;
  }

  // Triplicate state: 1/3 burn, 1/3 freeze, 1/3 paralyze
  // The first state (original) will be Burn
  // The second state (iREnv[1]) will be Freeze with prob 1/3
  // The third state (iREnv[2]) will be Paralysis with prob 1/3
  std::array<size_t, 3> iREnv;
  cu.triplicateState(iREnv, FixType(1.0 / 3.0), FixType(1.0 / 3.0));

  // State 0: Burn (Remaining probability = 1 - 1/3 - 1/3 = 1/3)
  cu.getTPKV(iREnv[0]).setStatusAilment(AIL_NV_BURN);

  // State 1: Freeze
  cu.getTPKV(iREnv[1]).setStatusAilment(AIL_NV_FREEZE);

  // State 2: Paralysis
  cu.getTPKV(iREnv[2]).setStatusAilment(AIL_NV_PARALYSIS);

  return 1;
}

void register_move_tri_attack(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnSecondaryEffect(move, "tri attack", move_tri_attack_secondary, 0, all_teams));
  // clang-format on
}

} // namespace gen4
