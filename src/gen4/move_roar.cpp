#include "gen4_scripts_internal.h"

namespace gen4 {

// Subclass PkCUEngine to access protected members
class RoarEngine : public PkCUEngine {
public:
 void setIBase(size_t i) { iBase_ = i; }
};

int move_roar_forceSwitch(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move* cMove = &mV.getBase();
  if (cMove != whirlwind_t && cMove != roar_t) { return 0; }
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  TeamVolatile tTV = cu.getTTV();

  // Find all valid switch-ins
  std::array<size_t, 6> validSwitchIns;
  size_t numOptions = 0;
  for (size_t i = 0; i < tTV.nv().getNumTeammates(); ++i) {
    if (i == tTV.getICPKV()) continue; // Current pokemon
    if (tTV.teammate(i).isAlive()) { validSwitchIns[numOptions++] = i; }
  }

  if (numOptions == 0) {
    // Move fails if no one to switch to
    return 0;
  }

  RoarEngine& rEngine = (RoarEngine&)cu;
  const size_t originalIBase = cu.getIBase();
  size_t currentEnvIndex = originalIBase;

  for (size_t i = 0; i < numOptions; ++i) {
    size_t envIndex;
    if (i < numOptions - 1) {
      std::array<size_t, 2> splitResult;
      FixType prob = FixType(1) / (int32_t)(numOptions - i);
      cu.duplicateState(splitResult, prob, currentEnvIndex);
      envIndex = splitResult[1];
      currentEnvIndex = splitResult[0];
    } else {
      envIndex = currentEnvIndex;
    }

    // Perform switch
    cu.getTTV(envIndex).swapPokemon(validSwitchIns[i]);
    cu.getStack().at(envIndex).setSwitched(cu.getIOTeam());
    PokemonVolatile newTPKV = cu.getTTV(envIndex).getPKV();

    // TODO(@drendleman): add support in PkCU for changing the stackstage via a
    // plugin call
    // Run PLUGIN_ON_SWITCHIN for the new state
    rEngine.setIBase(envIndex);
    cu.setCPluginSet();

    int result = 0;
    const std::vector<plugin_t>& cPlugins =
        cu.getCPluginSet()[(size_t)PLUGIN_ON_SWITCHIN];
    for (auto iPlugin = cPlugins.cbegin(), iPSize = cPlugins.cend();
         iPlugin != iPSize;
         ++iPlugin) {
      onSwitch_rawType cPlugin = (onSwitch_rawType)iPlugin->pFunction;
      result = result | cPlugin(cu, newTPKV);
      if (result > 1) { break; }
    }
  }

  rEngine.setIBase(originalIBase);
  cu.setCPluginSet();  // Restore

  return 2;
}

void register_move_roar(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(plugin(move, "roar", PLUGIN_ON_EVALUATEMOVE, move_roar_forceSwitch, 0, current_team));
  extensions.push_back(plugin(move, "whirlwind", PLUGIN_ON_EVALUATEMOVE, move_roar_forceSwitch, 0, current_team));
  // clang-format on
}

} // namespace gen4
