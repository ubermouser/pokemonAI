#include "gen4_scripts_internal.h"

namespace gen4 {

int move_torment_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != torment_t) { return 0; }

  // Fails if the target is already tormented.
  if (tPKV.status().cTeammate.torment) { return 1; }

  tPKV.status().cTeammate.torment = 1;
  return 1;
}

int move_torment_test(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  if (!cPKV.status().cTeammate.torment) { return 0; }

  // no effect if not a move:
  if (!action.isMove()) { return 0; }

  // Torment does not prevent Struggle
  if (action.isStruggle()) { return 0; }

  // Check last action
  // iLastAction is 1-based index of the move in the list + 1, so 0 means no action.
  // See engine_common.cpp: cPKV.status().cTeammate.iLastAction = lastAction.iMove() + 1;
  uint32_t iLastAction = cPKV.status().cTeammate.iLastAction;

  if (iLastAction > 0) {
      uint32_t lastMoveIndex = iLastAction - 1;
      if (action.iMove() == lastMoveIndex) {
          moveAllowed[VALID_MOVE_SCRIPT] = false;
      }
  }

  return 1;
}

void register_move_torment(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "torment", move_torment_set, 0, current_team));
  extensions.push_back(pluginOnTestMove(engine, "torment_test", move_torment_test, 0, all_teams));
}

} // namespace gen4
