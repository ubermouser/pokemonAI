#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_poison_heal_endOfRound(
    PkCUEngine& cu,
    const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (!cPKV.nv().abilityExists() || (&cPKV.nv().getAbility() != poisonHeal_t)) {
    return 0;
  }

  uint32_t condition = cPKV.getStatusAilment();
  if (condition == AIL_NV_POISON || condition == AIL_NV_POISON_TOXIC) {
    cPKV.modPercentHP(0.125);
    return 2; // Preempt engine damage effect
  }

  return 0;
}

void register_ability_poison_heal(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfRound(ability, "poison heal", ability_poison_heal_endOfRound, -1, current_team));
}

} // namespace gen4
