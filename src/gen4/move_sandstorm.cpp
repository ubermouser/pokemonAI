#include "gen4_scripts_internal.h"

namespace gen4 {

int move_sandstorm_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != sandstorm_t) { return 0; }

  auto& s = cu.getBase().getTeam(TEAM_A).status();
  if (s.weather_type == WEATHER_SAND) {
    return 1;
  }

  s.weather_type = WEATHER_SAND;
  s.weather_duration = 5;

  return 1;
}

void register_move_sandstorm(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "sandstorm", move_sandstorm_set, 0, current_team));
}

} // namespace gen4
