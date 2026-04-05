#include "gen4_scripts_internal.h"

namespace gen4 {

int move_encore_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != encore_t) { return 0; }

  // The last move used by the opponent
  uint32_t iLastAction = tPKV.status().cTeammate.iLastAction;
  bool switched = cu.getBase().flagsFor((TEAM)cu.getIOTeam()).isSwitched();
  if (iLastAction == 0 || switched) { return 1; }

  // Fails if the target is already under the effects of Encore.
  if (tPKV.status().cTeammate.encore_duration > 0) { return 1; }

  // iMove is 0-indexed index of the move in the pokemon's movelist
  const Move& oMove = tPKV.getMV(iLastAction - 1).getBase();

  // Illegal moves for Encore in Gen 4:
  // Encore, Mirror Move, Sketch, Mimic, Transform, Struggle.
  if (&oMove == encore_t || &oMove == struggle_t) { return 1; }

  tPKV.status().cTeammate.encore_action = iLastAction - 1;
  tPKV.status().cTeammate.encore_duration = 7;

  return 1;
}

int move_encore_test(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  if (cPKV.status().cTeammate.encore_duration == 0) { return 0; }

  // no effect if not a move:
  if (!action.isMove()) { return 0; }

  // only the encored move is allowed:
  uint32_t encAction = cPKV.status().cTeammate.encore_action;
  if (action.iMove() != encAction) { moveAllowed[VALID_MOVE_SCRIPT] = false; }

  return 1;
}

int move_encore_update(PkCUEngine& cu, PokemonVolatile cPKV) {
  auto& teamStatus = cPKV.status().cTeammate;
  if (teamStatus.encore_duration == 0) { return 0; }

  uint32_t duration = teamStatus.encore_duration;
  uint32_t encAction = teamStatus.encore_action;

  // Check if the encored move has PP
  if (!cPKV.getMV(Action::move(encAction)).hasPP()) {
    teamStatus.encore_duration = 0;
    teamStatus.encore_action = 0;
    return 1;
  }

  if (duration > 4) {
    teamStatus.encore_duration = duration - 1;
  } else if (duration == 1) {
    // Deterministic: Encore ends
    teamStatus.encore_duration = 0;
    teamStatus.encore_action = 0;
  } else {
    std::array<size_t, 2> iREnv;
    // Probability to end: 1/duration
    cu.duplicateState(iREnv, FixType(1.0f / duration));

    // Case 1: Encore ends
    {
      auto& newStatus = cu.getPKV(iREnv[0]).status().cTeammate;
      newStatus.encore_duration = 0;
      newStatus.encore_action = 0;
    }
    // Case 2: Encore continues
    {
      auto& newStatus = cu.getPKV(iREnv[1]).status().cTeammate;
      uint32_t new_duration = std::max((int32_t)duration - 1, 0);
      newStatus.encore_duration = new_duration;
      newStatus.encore_action = new_duration == 0 ? 0 : encAction;
    }
  }

  return 1;
}

int move_encore_preempt(PkCUEngine& cu, Action& action) {
  PokemonVolatile cPKV = cu.getPKV();
  auto& teamStatus = cPKV.status().cTeammate;
  if (teamStatus.encore_duration == 0) { return 0; }

  uint32_t encAction = teamStatus.encore_action;

  // If the action is a move and it's not the encored move, CHANGE it.
  if (action.isMove() && action.iMove() != encAction) {
    action = Action::move(encAction);
    return 1;
  }

  return 0;
}

void register_move_encore(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "encore", move_encore_set, 0, current_team));
  extensions.push_back(pluginOnTestMove(engine, "encore_test", move_encore_test, 0, all_teams));
  extensions.push_back(pluginOnModifyAction(engine, "encore_preempt", move_encore_preempt, 0, all_teams));
  extensions.push_back(pluginOnEndOfTurn(engine, "encore_update", move_encore_update, 0, all_teams));
}

} // namespace gen4
