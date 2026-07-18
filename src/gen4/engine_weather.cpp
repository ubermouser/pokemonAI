#include "gen4_scripts_internal.h"

namespace gen4 {


int weather_postRound(PkCUEngine& cu) {
  auto& s = cu.getBase().getTeam(TEAM_A).status();
  if (s.weather_duration > 0) {
    s.weather_duration--;
    if (s.weather_duration == 0) { s.weather_type = WEATHER_NORMAL; }
  }
  return 1;
}


int weather_endOfRound(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (!cPKV.isAlive()) { return 0; }

  auto& s = cu.getBase().getTeam(TEAM_A).status();
  if (s.weather_type == WEATHER_SAND) {
    // Check if immune by type:
    bool typeImmune = cPKV.nv().getBase().hasType(rock_t) ||
                      cPKV.nv().getBase().hasType(ground_t) ||
                      cPKV.nv().getBase().hasType(steel_t);
    if (typeImmune) { return 0; }

    // Check if immune by ability:
    const Ability* ability = &(cPKV.nv().getAbility());
    if (ability == sandVeil_t || ability == magicGuard_t) { return 0; }

    // Deal 1/16th max HP damage:
    uint32_t damage = cPKV.nv().getMaxHP() / 16;
    if (damage == 0) { damage = 1; }
    cPKV.modHP(-(int32_t)damage);
  }
  return 1;
}


int weather_modifyAttackPower(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  auto& s = cu.getBase().getTeam(TEAM_A).status();
  if (s.weather_type == WEATHER_SAND) {
    PokemonVolatile tPKV = cu.getPKV(target);
    if (tPKV.nv().getBase().hasType(rock_t)) {
      MoveVolatile mV = cu.getMV(actor);
      if (mV.getBase().getDamageType() == ATK_SPECIAL) {
        modifier *= (1.0 / 1.5);
      }
    }
  }
  return 1;
}


int dummy_ability(PokemonNonVolatile&, MoveNonVolatile&) { return 1; }


void register_engine_weather(
    const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnPostRound(engine, "weather post round", weather_postRound, 0, all_teams));
  extensions.push_back(pluginOnEndOfRound(engine, "weather end of round", weather_endOfRound, 10, all_teams));
  extensions.push_back(pluginOnModifyAttackPower(engine, "weather rock spdef boost", weather_modifyAttackPower, 0, all_teams));

  // Register dummy plugins for sandstorm-related abilities to mark them as implemented:
  extensions.push_back(pluginOnInit(ability, "magic guard", dummy_ability, 0, current_team));
  extensions.push_back(pluginOnInit(ability, "sand veil", dummy_ability, 0, current_team));
  // clang-format on
}

}  // namespace gen4
