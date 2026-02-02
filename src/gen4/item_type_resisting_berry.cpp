#include "gen4_scripts_internal.h"

namespace gen4 {


int item_typeResistingBerry(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& typeModifier) {
  if (!tPKV.hasItem()) { return 0; }

  const Type* cType = &mV.getBase().getType();
  const Type* resistedType = &tPKV.getItem().getResistedType();

  // no effect if attack type isn't of the resisted type
  if (cType != resistedType) { return 0; }

  // berry reduces the damage of the attack by 50%
  typeModifier *= 0.5;
  // berry consumed after use
  tPKV.setNoItem();

  return 1;
};

void register_item_type_resisting_berry(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(engine, "type resisting berry effect", PLUGIN_ON_MODIFYITEMPOWER, item_typeResistingBerry, 0, all_teams));
}

} // namespace gen4
