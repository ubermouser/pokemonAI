#ifndef ITEM_LUM_BERRY_CPP
#define ITEM_LUM_BERRY_CPP

#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int item_lumBerry(PkCUEngine& cu, PokemonVolatile cPKV) {
  // TODO(@drendleman) - why does this affect target pokemon and not current
  // pokemon?
  PokemonVolatile tPKV = cu.getTPKV();

  // only affect targeted pokemon that have a lum berry
  if (!tPKV.hasItem() || (&tPKV.getItem() != lumBerry_t)) { return 0; }

  // only affect living pokemon
  if (!tPKV.isAlive()) { return 0; }

  bool conditionCured = false;
  // volatile status condition confusion will be cured
  if (tPKV.status().cTeammate.confused > 0) {
    tPKV.status().cTeammate.confused = 0;
    conditionCured = true;
  }
  // all nonvolatile status conditions will be cured:
  else if (tPKV.getStatusAilment() != AIL_NV_NONE) {
    // cure status condition immediately
    tPKV.clearStatusAilment();
    conditionCured = true;
  }

  if (conditionCured) {
    tPKV.setNoItem();
    return 1;
  }
  return 0;
}


void register_item_lum_berry(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(item, "lum berry", PLUGIN_ON_ENDOFTURN, item_lumBerry, 0, all_teams));
}

} // namespace gen4

#endif
