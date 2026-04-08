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

  // u-turn performs no swap when the actor is the swap target (acceptable only
  // if no valid swap targets exist)
  if (action.iFriendly() == actor.iTeammate()) { return 1; }

#if USE_LEGACY_ENGINE
  cu.getBase().flagsFor(cu.getCActor()).setSwitched();
  tV.swapPokemon(action.iFriendly());
  cu.setCPluginSet();

  int result = 0;
  const auto& cPlugins = cu.getCPluginSet()[(size_t)PLUGIN_ON_SWITCHIN];
  for (const auto& plugin : cPlugins) {
    onSwitch_rawType cPlugin = (onSwitch_rawType)plugin.getFunction();
    result = result | cPlugin(cu, cu.getPKV());
    if (result > 1) { break; }
  }
#else

  auto& frame = cu.getStackFrame();

  frame.actions[actor] = Action::swap(action.iFriendly());
  frame.targets[actor] = {Actor(actor.iTeam(), action.iFriendly())};
  cu.gotoStackStage(StageType::PRESWITCH);
#endif

  return 2;
}


int move_uTurn_testMoveSwap(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  if (&mV.getBase() != uTurn_t) { return 0; }

  // normally, a friendly targeting move is disallowed when target friendly
  // pokemon is dead. But
  //  u-turn is allowed when there are no friendly pokemon.
  if (cTV.numTeammatesAlive() == 1) {
    moveAllowed[VALID_MOVE_FRIENDLY_IS_OTHER] = true;
  }

  return 1;
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
