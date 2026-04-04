#include "gen4_scripts_internal.h"

namespace gen4 {

int move_batonPass(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != batonPass_t) { return 0; }

  auto action = cu.getCAction();
  TeamVolatile tV = cu.getTV();
  size_t currentPokemon = tV.getICPKV();
  size_t targetPokemon = action.iFriendly();

  if (targetPokemon >= 6 || targetPokemon == currentPokemon) { return 1; }

  // Save Volatile Status
  VolatileStatus savedStatus = tV.getVolatile();

  // Perform Swap (reseting volatile)
  if (!tV.swapPokemon(targetPokemon, false)) {
    return 0; // Failed to swap
  }

  // Mark as switched
  cu.getBase().actor(cu.getCActor()).setSwitched();

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

void register_move_baton_pass(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "baton pass", PLUGIN_ON_ENDOFMOVE, move_batonPass, 1, current_team));
}

} // namespace gen4
