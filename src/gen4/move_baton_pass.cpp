#include "gen4_scripts_internal.h"

namespace gen4 {

void clearNonBatonPassVolatiles(VolatileStatus& status) {
  status.confused = 0;
  status.disable_duration = 0;
  status.disable_action = 0;
  status.healBlock = 0;
  status.encore_action = 0;
  status.encore_duration = 0;
  status.iLastAction = 0;
  status.flinch = 0;
  status.mudSport = 0;
  status.torment = 0;
  status.trap = 0;
  status.nightmare = 0;
  status.waterSport = 0;
  status.itemScratch = 0;
  status.numRoundsInPlay = 0;
  status.toxicPoison_tier = 0;
  status.lockIn_action = 0;
  status.lockIn_duration = 0;
  status.defensiveCurl = 0;
  status.imprison = 0;
  status.infatuate = 0;
  status.taunt_duration = 0;
  status.destinyBond = 0;
  status.grudge = 0;
  status.protect_counter = 0;
  status.protected_flag = 0;
}

int move_batonPass(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != batonPass_t) { return 0; }

  size_t currentPokemon = actor.iTeammate();
  size_t targetPokemon = action.iFriendly();

  if (targetPokemon >= 6 || targetPokemon == currentPokemon) { return 1; }
  if (!cu.getPKV(target).isAlive()) { return 1; }

  // Mark the actor as performing Baton Pass
  cu.getPKV(actor).status().batonPass = 1;

  // Schedule swap via the engine
  auto& frame = cu.getStackFrame();
  frame.actions[actor] = Action::swap(targetPokemon);
  frame.targets[actor] = {Actor(actor.iTeam(), targetPokemon)};
  cu.gotoStackStage(StageType::PRESWITCH);

  return 2;
}

int move_batonPass_onExecuteSwitch(
    PkCUEngine& cu,
    const Actor& actor,
    const Actor& target,
    bool& preserveVolatile) {
  PokemonVolatile pkv = cu.getPKV(actor);
  VolatileStatus& status = pkv.status();

  if (status.batonPass == 1) {
    // Clear batonPass flag to avoid leaking
    status.batonPass = 0;

    // Keep only transferable volatile status fields
    clearNonBatonPassVolatiles(status);

    // Tell the engine to preserve the status during swap
    preserveVolatile = true;
    return 2;  // handled
  }

  return 0;  // not handled
}

void register_move_baton_pass(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfMove(move, "baton pass", move_batonPass, 1, current_team));
  extensions.push_back(pluginOnExecuteSwitch(
      engine, "baton pass swap", move_batonPass_onExecuteSwitch, 0, all_teams));
}

} // namespace gen4
