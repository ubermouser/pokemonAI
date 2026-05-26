#include "gen4_scripts_internal.h"

namespace gen4 {

int move_batonPass(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != batonPass_t) { return 0; }

  TeamVolatile tV = cu.getTV();
  const Actor& currentActor = cu.getCActor();
  size_t currentPokemon = currentActor.iTeammate();
  size_t targetPokemon = action.iFriendly();

  if (targetPokemon >= 6 || targetPokemon == currentPokemon) { return 1; }

  // Save Volatile Status
  VolatileStatus savedStatus = tV.getVolatile();

  // Perform Swap (reseting volatile)
  Actor targetActor(currentActor.iTeam(), targetPokemon);
  if (!tV.swapPokemon(currentActor, targetActor, false)) {
    return 0; // Failed to swap
  }

  // Mark as switched
  cu.getBase().flagsFor(cu.getCActor()).setSwitched();

  // Restore transferrable volatile status
  VolatileStatus& newStatus = tV.getVolatile();

  // Boosts
  newStatus.boosts = savedStatus.boosts;

  // Other transferrables
  newStatus.substitute = savedStatus.substitute;
  newStatus.leechSeed = savedStatus.leechSeed;
  newStatus.perishSong = savedStatus.perishSong;
  newStatus.curse = savedStatus.curse;
  newStatus.lockOn = savedStatus.lockOn;
  newStatus.identify = savedStatus.identify;
  newStatus.focusEnergy = savedStatus.focusEnergy;
  // Ensure toxicPoison_tier is reset (already 0 from swapPokemon)

  // Trigger OnSwitchIn plugins
  cu.setCPluginSet();
  int result = 0;
  const auto& cPlugins = cu.getCPluginSet()[(size_t)PLUGIN_ON_SWITCHIN];
  for (const auto& plugin : cPlugins) {
    onSwitch_rawType cPlugin = (onSwitch_rawType)plugin.getFunction();
    result = result | cPlugin(cu, cu.getCActor());
    if (result > 1) { break; }
  }

  return 2;
}

void register_move_baton_pass(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfMove(move, "baton pass", move_batonPass, 1, current_team));
}

} // namespace gen4
