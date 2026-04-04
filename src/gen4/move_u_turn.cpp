#include "gen4_scripts_internal.h"

namespace gen4 {


int move_uTurn_swapOnTurnEnd(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != uTurn_t) { return 0; }
  auto action = cu.getCAction();
  TeamVolatile tV = cu.getTV();

  // u-turn is used when the current pokemon is the swap target (usable when no
  // other allies alive)
  if (action.iFriendly() == tV.getICPKV()) { return 1; }

  cu.getBase().actor(cu.getCActor()).setSwitched();
  tV.swapPokemon(action.iFriendly());
  cu.setCPluginSet();

  // TODO(@drendleman): add support in PkCU for changing the stackstage via a
  // plugin call
  int result = 0;
  const std::vector<plugin_t>& cPlugins =
      cu.getCPluginSet()[(size_t)PLUGIN_ON_SWITCHIN];
  for (auto iPlugin = cPlugins.cbegin(), iPSize = cPlugins.cend();
       iPlugin != iPSize;
       ++iPlugin) {
    onSwitch_rawType cPlugin = (onSwitch_rawType)iPlugin->pFunction;
    result = result | cPlugin(cu, cu.getPKV());
    if (result > 1) { break; }
  }

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

void register_move_u_turn(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "u-turn", PLUGIN_ON_ENDOFMOVE, move_uTurn_swapOnTurnEnd, 1, current_team));
  extensions.push_back(plugin(move, "u-turn", PLUGIN_ON_TESTMOVE, move_uTurn_testMoveSwap, 1, current_team));
}

} // namespace gen4
