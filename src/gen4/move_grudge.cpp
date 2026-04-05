#include "gen4_scripts_internal.h"

namespace gen4 {

int move_grudge(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != grudge_t) { return 0; }

  // Set grudge to active for the user
  cPKV.status().cTeammate.grudge = 1;

  return 1;
}

int move_grudge_clear(PkCUEngine& cu, PokemonVolatile cPKV) {
  cPKV.status().cTeammate.grudge = 0;
  return 0;  // Does not consume action
}

int move_grudge_trigger(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& damage) {
  // If the target (the one being attacked) has Grudge active
  if (!tPKV.status().cTeammate.grudge) { return 0; }

  // If the damage will not kill the target:
  if (damage < tPKV.getHP()) { return 1; }

  // The attacker fainted the target.
  // Reset the PP of the move used by the attacker.
  cu.getMV().setPP(0);
  return 1;
}

void register_move_grudge(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnEvaluateMove(move, "grudge", move_grudge, 0, current_team));
  extensions.push_back(pluginOnBeginningOfTurn(engine, "grudge_clear", move_grudge_clear, 0, all_teams));
  extensions.push_back(pluginOnCalculateDamage(engine, "grudge_trigger", move_grudge_trigger, 0, all_teams));
  // clang-format on
}

} // namespace gen4
