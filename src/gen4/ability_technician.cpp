#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int ability_technician(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& basePowerModifier) {
  if (!cPKV.nv().abilityExists() ||
      (&(cPKV.nv().getAbility()) != technician_t)) {
    return 0;
  }

  // no effect if base power above 60
  if (cu.getDamageComponent().damage > 60) { return 0; }

  // multiply base power by 1.5
  basePowerModifier *= 1.5;

  return 1;
};

void register_ability_technician(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "technician", PLUGIN_ON_MODIFYBASEPOWER, ability_technician, -1, current_team));
}

} // namespace gen4
