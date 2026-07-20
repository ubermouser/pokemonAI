#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_sand_stream_switch(
    PkCUEngine& cu,
    const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (&(cPKV.nv().getAbility()) != sandStream_t) { return 0; }

  auto& s = cu.getBase().getTeam(TEAM_A).status();
  if (s.weather_type == WEATHER_SAND) {
    return 0;
  }

  s.weather_type = WEATHER_SAND;
  // Nonstandard behavior: The engine cannot represent infinite weather duration.
  // Instead, we implement the Gen 6+ Sand Stream logic which sets the duration to 5 turns.
  s.weather_duration = 5;

  return 1;
}

int ability_sand_stream_beginning_of_game(
    EnvironmentVolatile& env,
    const Actor& actor) {
  PokemonVolatile cPKV = env.teammate(actor);
  if (&(cPKV.nv().getAbility()) != sandStream_t) { return 0; }

  auto& s = env.getTeam(TEAM_A).status();
  if (s.weather_type == WEATHER_SAND) {
    return 0;
  }

  s.weather_type = WEATHER_SAND;
  // Nonstandard behavior: The engine cannot represent infinite weather duration.
  // Instead, we implement the Gen 6+ Sand Stream logic which sets the duration to 5 turns.
  s.weather_duration = 5;

  return 1;
}

void register_ability_sand_stream(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnSwitchIn(ability, "sand stream", ability_sand_stream_switch, 1, current_team));
  extensions.push_back(pluginOnBeginningOfGame(ability, "sand stream", ability_sand_stream_beginning_of_game, 1, current_team));
}

} // namespace gen4
