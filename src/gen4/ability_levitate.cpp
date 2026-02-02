#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int ability_levitate(
    PkCUEngine& cu,
    const Type& cType,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& typeModifier) {
  if (!tPKV.nv().abilityExists() || (&(tPKV.nv().getAbility()) != levitate_t)) {
    return 0;
  }

  // no effect if attack type isn't ground
  if (&cType != ground_t) { return 0; }

  // make immune to ground type attack
  typeModifier *= 0.0;

  return 1;
};

int ability_levitate_switch(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (!cPKV.nv().abilityExists() || (&(cPKV.nv().getAbility()) != levitate_t)) {
    return 0;
  }

  // preempt scripts which deal damage on switchin
  return 2;
};


void register_ability_levitate(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "levitate", PLUGIN_ON_SETDEFENSETYPE, ability_levitate, -1, other_team));
  extensions.push_back(plugin(ability, "levitate", PLUGIN_ON_SWITCHIN, ability_levitate_switch, 1, current_team));
}

} // namespace gen4
