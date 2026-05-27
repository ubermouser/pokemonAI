#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_levitate(
    PkCUEngine& cu,
    const Type& cType,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& typeModifier) {
  PokemonVolatile tPKV = cu.getPKV(target);
  if (&(tPKV.nv().getAbility()) != levitate_t) { return 0; }

  // no effect if attack type isn't ground
  if (&cType != ground_t) { return 0; }

  // make immune to ground type attack
  typeModifier *= 0.0;

  return 1;
};

int ability_levitate_switch(
    PkCUEngine& cu,
    const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (&(cPKV.nv().getAbility()) != levitate_t) { return 0; }

  // preempt scripts which deal damage on switchin
  return 2;
};


void register_ability_levitate(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnSetDefenseType(ability, "levitate", ability_levitate, -1, other_team));
  extensions.push_back(pluginOnSwitchIn(ability, "levitate", ability_levitate_switch, 1, current_team));
}

} // namespace gen4
