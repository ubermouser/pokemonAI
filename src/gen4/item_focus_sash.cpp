#ifndef ITEM_FOCUS_SASH_CPP
#define ITEM_FOCUS_SASH_CPP

#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {


int item_focusSash(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& raw_damage) {
  if (!tPKV.hasItem() || (&tPKV.getItem() != focusSash_t)) { return 0; }

  // Focus Sash only works if HP is full
  if (tPKV.getHP() != tPKV.nv().getMaxHP()) { return 0; }

  // Focus Sash prevents OHKO
  if (raw_damage >= tPKV.getHP()) {
    raw_damage = tPKV.getHP() - 1;
    tPKV.setNoItem();
    return 1;
  }

  return 0;
}

void register_item_focus_sash(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(item, "focus sash", PLUGIN_ON_CALCULATEDAMAGE, item_focusSash, 0, all_teams));
}

} // namespace gen4

#endif
