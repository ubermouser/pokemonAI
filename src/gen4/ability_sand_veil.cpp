#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_sand_veil(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    FixType& probabilityToHit) {
  PokemonVolatile tPKV = cu.getPKV(target);
  if (&(tPKV.nv().getAbility()) != sandVeil_t) { return 0; }

  auto& s = cu.getBase().getTeam(TEAM_A).status();
  if (s.weather_type != WEATHER_SAND) { return 0; }

  probabilityToHit *= FixType(4, 5);
  return 1;
}

void register_ability_sand_veil(
    const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyHitProbability(
      ability, "sand veil", ability_sand_veil, 0, other_team));
}

}  // namespace gen4
