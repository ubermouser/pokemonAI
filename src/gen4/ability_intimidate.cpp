#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_intimidate_switch(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (!cPKV.nv().abilityExists() ||
      (&(cPKV.nv().getAbility()) != intimidate_t)) {
    return 0;
  }

  // affects the opponent:
  PokemonVolatile tPKV = cu.getTPKV();
  if (!tPKV.isAlive()) { return 0; }

  // blocked by Clear Body:
  if (tPKV.nv().abilityExists() && (&(tPKV.nv().getAbility()) == clearBody_t)) {
    return 0;
  }

  // lowers attack by 1 stage:
  tPKV.modBoost(FV_ATTACK, -1);

  return 1;
};

void register_ability_intimidate(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnSwitchIn(ability, "intimidate", ability_intimidate_switch, 1, current_team));
}

} // namespace gen4
