#include "gen4_scripts_internal.h"

namespace gen4 {


int item_focusSash(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    uint32_t& raw_damage) {
  PokemonVolatile tPKV = cu.getPKV(target);
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
  extensions.push_back(pluginOnCalculateDamage(item, "focus sash", item_focusSash, 0, all_teams));
}

} // namespace gen4
