#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_noGuard(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToHit) {
  bool doNoGuard = false;
  if ((cPKV.nv().abilityExists() && (&(cPKV.nv().getAbility()) == noGuard_t)) ||
      (tPKV.nv().abilityExists() && (&(tPKV.nv().getAbility()) == noGuard_t))) {
    doNoGuard = true;
  }

  if (!doNoGuard) { return 0; }

  probabilityToHit = FixType(1.0);

  // do not allow anything to affect hit chance other than this if no guard
  // occurs;
  return 2;
};

void register_ability_no_guard(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyHitProbability(ability, "no guard", ability_noGuard, -2, all_teams));
}

} // namespace gen4
