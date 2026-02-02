#include "gen4_scripts_internal.h"

namespace gen4 {

// Subclass PkCUEngine to access protected members
class RoarEngine : public PkCUEngine {
public:
  void setIBase(size_t i) { iBase_ = i; }
  const PkCU& getCU() const { return cu_; }
};

int move_roar_forceSwitch(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {

  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  TeamVolatile tTV = cu.getTTV();

  // Find all valid switch-ins
  std::vector<size_t> validSwitchIns;
  for (size_t i = 0; i < tTV.nv().getNumTeammates(); ++i) {
    if (i == tTV.getICPKV()) continue; // Current pokemon
    if (tTV.teammate(i).isAlive()) {
      validSwitchIns.push_back(i);
    }
  }

  if (validSwitchIns.empty()) {
    // Move fails if no one to switch to
    return 0;
  }

  size_t numOptions = validSwitchIns.size();
  size_t currentEnvIndex = cu.getIBase();

  // Split state if more than one option
  std::array<size_t, 2> splitResult;

  for (size_t i = 0; i < numOptions - 1; ++i) {
      FixType prob = FixType(1) / (int32_t)(numOptions - i);

      cu.duplicateState(splitResult, prob, currentEnvIndex);

      size_t envIndex = splitResult[1];

      // Perform switch
      cu.getTTV(envIndex).swapPokemon(validSwitchIns[i]);

      PokemonVolatile newTPKV = cu.getTTV(envIndex).getPKV();

      // Run PLUGIN_ON_SWITCHIN for the new state
      RoarEngine& rEngine = (RoarEngine&)cu;
      size_t originalIBase = cu.getIBase();
      rEngine.setIBase(envIndex);
      cu.setCPluginSet();

      int result = 0;
      const std::vector<plugin_t>& cPlugins = cu.getCPluginSet()[(size_t)PLUGIN_ON_SWITCHIN];
      for (auto iPlugin = cPlugins.cbegin(), iPSize = cPlugins.cend(); iPlugin != iPSize; ++iPlugin) {
          onSwitch_rawType cPlugin = (onSwitch_rawType)iPlugin->pFunction;
          result = result | cPlugin(cu, newTPKV);
          if (result > 1) { break; }
      }

      rEngine.setIBase(originalIBase);
      cu.setCPluginSet(); // Restore

      // Update currentEnvIndex to be the remaining state
      currentEnvIndex = splitResult[0];
  }

  // Handle the last option (currentEnvIndex)
  cu.getTTV(currentEnvIndex).swapPokemon(validSwitchIns.back());

  PokemonVolatile newTPKV = cu.getTTV(currentEnvIndex).getPKV();
  int result = 0;

  cu.setCPluginSet();

  const std::vector<plugin_t>& cPlugins = cu.getCPluginSet()[(size_t)PLUGIN_ON_SWITCHIN];
  for (auto iPlugin = cPlugins.cbegin(), iPSize = cPlugins.cend(); iPlugin != iPSize; ++iPlugin) {
      onSwitch_rawType cPlugin = (onSwitch_rawType)iPlugin->pFunction;
      result = result | cPlugin(cu, newTPKV);
      if (result > 1) { break; }
  }

  return 2;
}

void register_move_roar(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "roar", PLUGIN_ON_ENDOFMOVE, move_roar_forceSwitch, 2, current_team));
  extensions.push_back(plugin(move, "whirlwind", PLUGIN_ON_ENDOFMOVE, move_roar_forceSwitch, 2, current_team));
}

} // namespace gen4
