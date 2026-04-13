#include "gen4_scripts_internal.h"

namespace gen4 {


int move_uTurn_swapOnTurnEnd(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != uTurn_t) { return 0; }
  auto actor = cu.getCActor();
  auto action = cu.getCAction();
  TeamVolatile tV = cu.getTV();

  // u-turn performs no swap when the actor has no friendly target specified,
  // or when the actor is the swap target (acceptable only if no valid swap
  // targets exist)
  if (!action.targetedFriendly() || action.iFriendly() == actor.iTeammate()) {
    return 1;
  }

  auto& frame = cu.getStackFrame();

  frame.actions[actor] = Action::swap(action.iFriendly());
  frame.targets[actor] = {Actor(actor.iTeam(), action.iFriendly())};
  cu.gotoStackStage(StageType::PRESWITCH);

  return 2;
}


int move_uTurn_testMoveSwap(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  if (&mV.getBase() != uTurn_t) { return 0; }

  if (!action.targetedFriendly()) {
    moveAllowed[VALID_MOVE_SCRIPT] = false;
  }

  // Allow self-targeting for U-turn as it handles its own swap logic.
  moveAllowed[VALID_MOVE_TARGET_IS_OTHER] = true;

  return 2;
}


int move_uTurn_modifyTarget(PkCUEngine& cu, Action& action) {
  auto actor = cu.getCActor();
  auto& frame = cu.getStackFrame();
  Action attackAction = Action::move(action.iMove());
  frame.targets[actor] = {
      cu.getBase().getEnv().defaultTarget(actor, attackAction)};
  return 1;
}


void register_move_u_turn(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnEndOfMove(move, "u-turn", move_uTurn_swapOnTurnEnd, 1, current_team));
  extensions.push_back(pluginOnTestMove(move, "u-turn", move_uTurn_testMoveSwap, 1, current_team));
  extensions.push_back(pluginOnModifyAction(move, "u-turn", move_uTurn_modifyTarget, 1, current_team));
  // clang-format on
}

} // namespace gen4
