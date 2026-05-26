#include "gen4_scripts_internal.h"

namespace gen4 {

int item_leftovers(
    PkCUEngine& cu,
    const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (cPKV.hasItem() && (&cPKV.getItem() == leftovers_t)) {
    cPKV.modPercentHP(0.0625);
    return 1;
  }

  return 0;
};

void register_item_leftovers(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfRound(item, "leftovers", item_leftovers, 0, all_teams));
}

} // namespace gen4
