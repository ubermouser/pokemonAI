#include "gen4_scripts_internal.h"

namespace gen4 {

int item_toxic_orb(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (cPKV.hasItem() && (&cPKV.getItem() == toxicOrb_t)) {
    if (cPKV.getStatusAilment() == AIL_NV_NONE) {
        // Toxic Orb does not affect Poison or Steel types
        const Type* type1 = &cPKV.getBase().getType(0);
        const Type* type2 = &cPKV.getBase().getType(1);

        if (type1 == poison_t || type2 == poison_t || type1 == steel_t || type2 == steel_t) {
            return 0;
        }

        cPKV.setStatusAilment(AIL_NV_POISON_TOXIC);
        return 1;
    }
  }

  return 0;
};

void register_item_toxic_orb(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(item, "toxic orb", PLUGIN_ON_ENDOFROUND, item_toxic_orb, 0, all_teams));
}

} // namespace gen4
