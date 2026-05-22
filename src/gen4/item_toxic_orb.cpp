#include "gen4_scripts_internal.h"

namespace gen4 {

int item_toxic_orb(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (cPKV.hasItem() && (&cPKV.getItem() == toxicOrb_t)) {
    // Toxic Orb does not affect Poison or Steel types
    const PokemonBase& base = cPKV.getBase();
    if (base.hasType(poison_t) || base.hasType(steel_t)) {
        return 0;
    }

    // Toxic Orb does not activate if the pokemon already has a non-volatile status ailment.
    if (cPKV.getStatusAilment() != AIL_NV_NONE) {
        return 0;
    }

    // Toxic Orb induces Bad Poison at the end of the turn.
    cPKV.setStatusAilment(AIL_NV_POISON_TOXIC);
    return 1;
  }

  return 0;
};

void register_item_toxic_orb(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfRound(item, "toxic orb", item_toxic_orb, 0, current_team));
}

} // namespace gen4
