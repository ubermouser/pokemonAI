#include "gen4_scripts_internal.h"
#include <algorithm>

namespace gen4 {

int move_ohko_accuracy(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToHit) {
  const Move* tMove = &mV.getBase();
  bool isFissure = (tMove == fissure_t);
  bool isSheerCold = (tMove == sheer_cold_t);
  bool isGuillotine = (tMove == guillotine_t);
  bool isHornDrill = (tMove == horn_drill_t);

  if (!isFissure && !isSheerCold && !isGuillotine && !isHornDrill) {
    return 0;
  }

  // OHKO moves fail if target level > user level
  if (tPKV.nv().getLevel() > cPKV.nv().getLevel()) {
    probabilityToHit = FixType(0.0f);
    return 1;
  }

  // Sheer Cold Immunities (Ice Type)
  if (isSheerCold) {
    const PokemonBase& tPKB = tPKV.getBase();
    if ((&tPKB.getType(0) == ice_t) || (&tPKB.getType(1) == ice_t)) {
      probabilityToHit = FixType(0.0f);
      return 1;
    }
  }

  // NOTE: Fissure immunity (Flying/Levitate) and Guillotine/HornDrill immunity (Ghost)
  // are handled by standard type effectiveness logic in engine (PLUGIN_ON_SETDEFENSETYPE),
  // which sets damage to 0. We respect 0 damage in move_ohko_damage.

  // Calculate Accuracy: ((UserLevel - TargetLevel) + 30)%
  int32_t acc = (cPKV.nv().getLevel() - tPKV.nv().getLevel()) + 30;
  if (acc >= 100) {
    probabilityToHit = FixType(1.0f);
  } else if (acc <= 0) {
    probabilityToHit = FixType(0.0f);
  } else {
    probabilityToHit = FixType(acc) / FixType(100.0f);
  }

  return 1;
}

int move_ohko_base_power(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& basePower) {
  const Move* tMove = &mV.getBase();
  if (tMove != fissure_t && tMove != sheer_cold_t && tMove != guillotine_t && tMove != horn_drill_t) {
    return 0;
  }

  // Set dummy base power to pass engine assertion (basePower > 0)
  basePower = 1;
  return 1;
}

int move_ohko_damage(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& damage) {
  const Move* tMove = &mV.getBase();
  if (tMove != fissure_t && tMove != sheer_cold_t && tMove != guillotine_t && tMove != horn_drill_t) {
    return 0;
  }

  // If damage is already 0 (e.g. immunity), do not override.
  if (damage == 0) {
    return 0;
  }

  // OHKO: Damage = Target's Current HP
  damage = tPKV.getHP();
  return 1;
}

void register_move_ohko(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // Register for Fissure
  if (fissure_t) {
    extensions.push_back(plugin(move, "fissure", PLUGIN_ON_MODIFYHITPROBABILITY, move_ohko_accuracy, 0, current_team));
    extensions.push_back(plugin(move, "fissure", PLUGIN_ON_SETBASEPOWER, move_ohko_base_power, 0, current_team));
    extensions.push_back(plugin(move, "fissure", PLUGIN_ON_CALCULATEDAMAGE, move_ohko_damage, 0, current_team));
  }
  // Register for Sheer Cold
  if (sheer_cold_t) {
    extensions.push_back(plugin(move, "sheer cold", PLUGIN_ON_MODIFYHITPROBABILITY, move_ohko_accuracy, 0, current_team));
    extensions.push_back(plugin(move, "sheer cold", PLUGIN_ON_SETBASEPOWER, move_ohko_base_power, 0, current_team));
    extensions.push_back(plugin(move, "sheer cold", PLUGIN_ON_CALCULATEDAMAGE, move_ohko_damage, 0, current_team));
  }
  // Register for Guillotine
  if (guillotine_t) {
    extensions.push_back(plugin(move, "guillotine", PLUGIN_ON_MODIFYHITPROBABILITY, move_ohko_accuracy, 0, current_team));
    extensions.push_back(plugin(move, "guillotine", PLUGIN_ON_SETBASEPOWER, move_ohko_base_power, 0, current_team));
    extensions.push_back(plugin(move, "guillotine", PLUGIN_ON_CALCULATEDAMAGE, move_ohko_damage, 0, current_team));
  }
  // Register for Horn Drill
  if (horn_drill_t) {
    extensions.push_back(plugin(move, "horn drill", PLUGIN_ON_MODIFYHITPROBABILITY, move_ohko_accuracy, 0, current_team));
    extensions.push_back(plugin(move, "horn drill", PLUGIN_ON_SETBASEPOWER, move_ohko_base_power, 0, current_team));
    extensions.push_back(plugin(move, "horn drill", PLUGIN_ON_CALCULATEDAMAGE, move_ohko_damage, 0, current_team));
  }
}

} // namespace gen4
