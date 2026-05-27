#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_technician(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& basePowerModifier) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (&(cPKV.nv().getAbility()) != technician_t) { return 0; }

  // no effect if base power above 60
  if (cu.getDamageComponent().damage > 60) { return 0; }

  // multiply base power by 1.5
  basePowerModifier *= 1.5;

  return 1;
};

void register_ability_technician(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyBasePower(ability, "technician", ability_technician, -1, current_team));
}

} // namespace gen4
