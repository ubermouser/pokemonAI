#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_intimidate_switch(
    PkCUEngine& cu,
    const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (&(cPKV.nv().getAbility()) != intimidate_t) { return 0; }

  int ret = 0;
  // affects all active opponents:
  for (auto [actor, tPKV] :
       cu.getBase().getTeam(cu.getIOTeam()).yieldActivePokemon()) {
    if (!tPKV.isAlive()) { continue; }

    // blocked by Clear Body:
    if (&(tPKV.nv().getAbility()) == clearBody_t) { continue; }

    // lowers attack by 1 stage:
    tPKV.modBoost(FV_ATTACK, -1);
    ret = 1;
  }

  return ret;
};

void register_ability_intimidate(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnSwitchIn(ability, "intimidate", ability_intimidate_switch, 1, current_team));
}

} // namespace gen4
