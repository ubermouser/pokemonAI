#include "gen4_scripts_internal.h"

namespace gen4 {

int move_curse_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != curse_t) { return 0; }

  // Check if user is Ghost type
  if (cPKV.getBase().hasType(ghost_t)) {
    // Fails if target already cursed
    if (tPKV.status().cTeammate.curse) { return 0; }

    // Apply Curse status to target
    tPKV.status().cTeammate.curse = 1;

    // User loses 1/2 of max HP
    cPKV.modPercentHP(-0.5);
  } else {
    // Non-Ghost type: Lower Speed, Raise Attack, Raise Defense
    cPKV.modBoost(FV_SPEED, -1);
    cPKV.modBoost(FV_ATTACK, 1);
    cPKV.modBoost(FV_DEFENSE, 1);
  }

  return 1;
}

int move_curse_effect(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (!cPKV.status().cTeammate.curse) { return 0; }

  // Cursed pokemon loses 1/4 of max HP
  cPKV.modPercentHP(-0.25);

  return (cPKV.isAlive() ? 1 : 2);
}

void register_move_curse(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "curse", move_curse_set, 0, current_team));
  extensions.push_back(pluginOnEndOfRound(move, "curse", move_curse_effect, 0, all_teams));
}

} // namespace gen4
