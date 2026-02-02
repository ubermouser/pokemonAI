#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int move_outrage_lockMove(PkCUEngine& cu, PokemonVolatile cPKV) {
  // action is guaranteed to be a move action:
  MoveVolatile mV = cPKV.getMV(cu.getCAction());
  auto& status = cPKV.status();
  // if not outrage, ignore:
  if (&mV.getBase() != outrage_t) { return 0; }
  // outrage cannot be re-locked if it is currently locked in:
  if (status.cTeammate.lockIn_duration > 0) { return 0; }

  size_t action_idx = cu.getCAction().iMove() + 1;
  status.cTeammate.lockIn_duration = 3;
  status.cTeammate.lockIn_action = action_idx;

  return 1;
}

int move_outrage_endLockOn(PkCUEngine& cu, PokemonVolatile cPKV) {
  // are we locked in to outrage?
  auto& status = cPKV.status();
  if (status.cTeammate.lockIn_duration == 0) { return 0; }
  MoveVolatile mV = cPKV.getMV(status.cTeammate.lockIn_action - 1);
  if (&mV.getBase() != outrage_t) { return 0; }
  // if the enemy team has a free move, do not decrement lock-on counter
  if (cu.getBase().hasWaited(cu.getICTeam())) { return 0; }

  // 50% chance to end at stage 1:
  if (status.cTeammate.lockIn_duration == 2) {
    std::array<size_t, 2> iREnv;
    cu.duplicateState(iREnv, FixType(0.5f));

    PokemonVolatile rPKV = cu.getPKV(iREnv[1]);
    // state #1: pokemon snaps out of dragon dance immediatelay and becomes
    // confused:
    rPKV.status().cTeammate.lockIn_duration = 0;
    rPKV.status().cTeammate.lockIn_action = 0;
    rPKV.status().cTeammate.confused = AIL_V_CONFUSED_5T;
  }
  // state #2 / else: dragon dance counts down for another turn:
  status.cTeammate.lockIn_duration--;

  // if this was the last dragon dance stage, confuse the pokemon:
  if (status.cTeammate.lockIn_duration == 0) {
    status.cTeammate.confused = AIL_V_CONFUSED_5T;
    status.cTeammate.lockIn_action = 0;
  }
  return 1;
}

int move_testLockedIn(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  if (cPKV.status().cTeammate.lockIn_duration == 0) { return 0; }

  // if locked in, only the locked-in move may be used. Other move actions are
  // not permitted.

  size_t action_idx = action.iMove() + 1;
  moveAllowed[VALID_MOVE_SCRIPT] =
      moveAllowed[VALID_MOVE_SCRIPT] &
      (cPKV.status().cTeammate.lockIn_action == action_idx);

  return 1;
}

int move_testLockedSwitch(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile cOPKV,
    const Action& action,
    ValidSwapSet& switchAllowed) {
  if (cPKV.status().cTeammate.lockIn_duration == 0) { return 0; }

  // if locked in, only the locked-in move may be used. Switch actions are not
  // permitted.
  switchAllowed[VALID_SWAP_SCRIPT] = false;

  return 1;
}

void register_move_outrage(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "outrage", PLUGIN_ON_ENDOFTURN, move_outrage_endLockOn, 0, current_team));
  extensions.push_back(plugin(move, "outrage", PLUGIN_ON_TESTMOVE, move_testLockedIn, 0, current_team));
  extensions.push_back(plugin(move, "outrage", PLUGIN_ON_TESTSWITCH, move_testLockedSwitch, 0, current_team));
  extensions.push_back(plugin(move, "outrage", PLUGIN_ON_BEGINNINGOFTURN, move_outrage_lockMove, 0, current_team));
}

} // namespace gen4
