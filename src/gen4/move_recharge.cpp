#include "gen4_scripts_internal.h"

namespace gen4 {

/**
 * @brief Recharge action index for lockIn_action.
 * Values 1-4 are reserved for move indexes.
 * Values 5-7 are available for other lock-in effects.
 */
const uint32_t ACTION_RECHARGE = 7;

/**
 * @brief Plugin triggered after a move is executed.
 * If the move was a recharge move and it hit, it sets the recharge status.
 */
int move_recharge_onEndMove(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // Recharge only occurs if the move hit.
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  // Set recharge status.
  // lockIn_duration = 2 because it will be decremented at the end of the current turn.
  auto& status = cPKV.status();
  status.cTeammate.lockIn_duration = 2;
  status.cTeammate.lockIn_action = ACTION_RECHARGE;

  return 1;
}

/**
 * @brief Plugin triggered at the beginning of a turn.
 * If the pokemon is recharging, it blocks its action for the turn.
 */
int move_recharge_onBeginTurn(PkCUEngine& cu, PokemonVolatile cPKV) {
  auto& status = cPKV.status();
  if (status.cTeammate.lockIn_action == ACTION_RECHARGE && status.cTeammate.lockIn_duration > 0) {
    // If the pokemon just used the recharge move this turn, duration is still 2 (before end-of-turn decrement).
    // We only want to block the NEXT turn, when duration is 1.
    if (status.cTeammate.lockIn_duration == 1) {
      cu.getBase().setBlocked(cu.getICTeam());
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Plugin triggered at the end of a turn.
 * Decrements the recharge duration and clears the status when it reaches zero.
 */
int move_recharge_onEndTurn(PkCUEngine& cu, PokemonVolatile cPKV) {
  auto& status = cPKV.status();
  if (status.cTeammate.lockIn_action == ACTION_RECHARGE) {
    if (status.cTeammate.lockIn_duration > 0) {
      status.cTeammate.lockIn_duration--;
    }
    if (status.cTeammate.lockIn_duration == 0) {
      status.cTeammate.lockIn_action = 0;
    }
    return 1;
  }
  return 0;
}

/**
 * @brief Plugin to check if a move is valid.
 * Disallows all moves if the pokemon is recharging.
 */
int move_recharge_testMove(
    ConstTeamVolatile cTV,
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    ValidMoveSet& moveAllowed) {
  if (cPKV.status().cTeammate.lockIn_action == ACTION_RECHARGE && cPKV.status().cTeammate.lockIn_duration == 1) {
    moveAllowed[VALID_MOVE_SCRIPT] = false;
    return 1;
  }
  return 0;
}

/**
 * @brief Plugin to check if a switch is valid.
 * Disallows switching if the pokemon is recharging.
 */
int move_recharge_testSwitch(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile cOPKV,
    const Action& action,
    ValidSwapSet& switchAllowed) {
  if (cPKV.status().cTeammate.lockIn_action == ACTION_RECHARGE && cPKV.status().cTeammate.lockIn_duration == 1) {
    switchAllowed[VALID_SWAP_SCRIPT] = false;
    return 1;
  }
  return 0;
}

/**
 * @brief Registers the recharge moves and their shared implementation.
 */
void register_move_recharge(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  const std::vector<std::string> moves = {
    "blast burn", "frenzy plant", "giga impact", "hydro cannon", "hyper beam", "rock wrecker"
  };

  for (const auto& moveName : moves) {
    extensions.push_back(plugin(move, moveName, PLUGIN_ON_ENDOFMOVE, move_recharge_onEndMove, 0, current_team));
  }

  // Register engine-level plugins to handle the recharge state.
  // These will be called for all pokemon but will only act if the recharge flag is set.
  extensions.push_back(plugin(engine, "recharge_begin_turn", PLUGIN_ON_BEGINNINGOFTURN, move_recharge_onBeginTurn, 0, all_teams));
  extensions.push_back(plugin(engine, "recharge_end_turn", PLUGIN_ON_ENDOFTURN, move_recharge_onEndTurn, 0, all_teams));
  extensions.push_back(plugin(engine, "recharge_test_move", PLUGIN_ON_TESTMOVE, move_recharge_testMove, 0, all_teams));
  extensions.push_back(plugin(engine, "recharge_test_switch", PLUGIN_ON_TESTSWITCH, move_recharge_testSwitch, 0, all_teams));
}

} // namespace gen4
