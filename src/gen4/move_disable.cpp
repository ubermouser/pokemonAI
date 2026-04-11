#include "gen4_scripts_internal.h"

namespace gen4 {

int move_disable_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != disable_t) { return 0; }

  // The last move used by the opponent
  uint32_t iLastAction = tPKV.status().cTeammate.iLastAction;
  if (iLastAction == 0) { return 1; }

  // Fails if the target is already disabled
  if (tPKV.status().cTeammate.disable_duration > 0) { return 1; }

  // iMove is 0-indexed index of the move in the pokemon's movelist
  const Move& oMove = tPKV.getMV(iLastAction - 1).getBase();

  // Illegal moves for Disable:
  // Struggle.
  if (&oMove == struggle_t) { return 1; }

  tPKV.status().cTeammate.disable_action = iLastAction - 1;
  tPKV.status().cTeammate.disable_duration = 7;

  return 1;
}

int move_disable_test(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  if (cPKV.status().cTeammate.disable_duration == 0) { return 0; }

  // no effect if not a move:
  if (!action.isMove()) { return 0; }

  // if the move is the disabled move, forbid it
  uint32_t disAction = cPKV.status().cTeammate.disable_action;
  if (action.iMove() == disAction) { moveAllowed[VALID_MOVE_SCRIPT] = false; }

  return 1;
}

int move_disable_update(PkCUEngine& cu, PokemonVolatile cPKV) {
  auto& teamStatus = cPKV.status().cTeammate;
  if (teamStatus.disable_duration == 0) { return 0; }

  uint32_t duration = teamStatus.disable_duration;

  if (duration > 4) {
    teamStatus.disable_duration = duration - 1;
  } else if (duration == 1) {
    // Deterministic: Disable ends
    teamStatus.disable_duration = 0;
    teamStatus.disable_action = 0;
  } else {
    std::array<size_t, 2> iREnv;
    // Probability to end: 1/duration
    cu.duplicateState(iREnv, FixType(1.0f / duration));

    // Case 1: Disable ends
    {
      auto& newStatus = cu.getPKV(iREnv[0]).status().cTeammate;
      newStatus.disable_duration = 0;
      newStatus.disable_action = 0;
    }
    // Case 2: Disable continues
    {
      auto& newStatus = cu.getPKV(iREnv[1]).status().cTeammate;
      uint32_t new_duration = std::max((int32_t)duration - 1, 0);
      newStatus.disable_duration = new_duration;
      newStatus.disable_action = new_duration == 0 ? 0 : teamStatus.disable_action;
    }
  }

  return 1;
}

void register_move_disable(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "disable", move_disable_set, 0, current_team));
  extensions.push_back(pluginOnTestMove(engine, "disable_test", move_disable_test, 0, all_teams));
  extensions.push_back(pluginOnEndOfRound(engine, "disable_update", move_disable_update, 0, all_teams));
}

} // namespace gen4
