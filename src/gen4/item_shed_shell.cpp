#include "gen4_scripts_internal.h"

namespace gen4 {


int item_shedShell_allowSwitch(
    ConstPokemonVolatile cPKV,
    ConstPokemonVolatile fPKV,
    ConstPokemonVolatile tPKV,
    const Action& action,
    ValidSwapSet& switchAllowed) {
  if (cPKV.hasItem() && (&cPKV.getItem() == shedShell_t)) {
    return 2;
  }

  return 0;
}

void register_item_shed_shell(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(item, "shed shell", PLUGIN_ON_TESTSWITCH, item_shedShell_allowSwitch, -100, current_team));
}

} // namespace gen4
