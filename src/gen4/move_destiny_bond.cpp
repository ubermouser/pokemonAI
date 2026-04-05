#include "gen4_scripts_internal.h"

namespace gen4 {

int move_destinyBond(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != destinyBond_t) { return 0; }

  // Fails if used consecutively in Gen 4
  uint32_t iLastAction = cPKV.status().cTeammate.iLastAction;
  if (iLastAction > 0) {
    const Move& lastMove = cPKV.getMV(iLastAction - 1).getBase();
    if (&lastMove == destinyBond_t) { return 1; }
  }

  cPKV.status().cTeammate.destinyBond = 1;

  return 1;
}

int move_destinyBond_clear(PkCUEngine& cu, PokemonVolatile cPKV) {
  cPKV.status().cTeammate.destinyBond = 0;
  return 0;  // Does not consume action
}

int move_destinyBond_trigger(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& damage) {
  // If the target has Destiny Bond active
  if (tPKV.status().cTeammate.destinyBond) {
    // If the damage will kill the target
    if (damage >= tPKV.getHP()) {
      // The attacker faints too
      cPKV.setHP(0);
      return 1;
    }
  }
  return 0;
}

void register_move_destiny_bond(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "destiny bond", move_destinyBond, 0, current_team));
  extensions.push_back(pluginOnBeginningOfTurn(engine, "destiny_bond_clear", move_destinyBond_clear, 0, all_teams));
  extensions.push_back(pluginOnCalculateDamage(engine, "destiny_bond_trigger", move_destinyBond_trigger, 0, all_teams));
}

} // namespace gen4
