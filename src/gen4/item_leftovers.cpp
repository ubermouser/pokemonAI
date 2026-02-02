#ifndef ITEM_LEFTOVERS_CPP
#define ITEM_LEFTOVERS_CPP

#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int item_leftovers(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (cPKV.hasItem() && (&cPKV.getItem() == leftovers_t)) {
    cPKV.modPercentHP(0.0625);
    return 1;
  }

  return 0;
};

void register_item_leftovers(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(item, "leftovers", PLUGIN_ON_ENDOFROUND, item_leftovers, 0, all_teams));
}

} // namespace gen4

#endif
