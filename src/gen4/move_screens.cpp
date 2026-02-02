#include "gen4_scripts_internal.h"

namespace gen4 {

int move_reflect_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != reflect_t) { return 0; }

  if (cPKV.status().nonvolatile.reflect > 0) { return 1; }

  cPKV.status().nonvolatile.reflect = 5;
  return 1;
}

int move_lightScreen_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != lightScreen_t) { return 0; }

  if (cPKV.status().nonvolatile.lightScreen > 0) { return 1; }

  cPKV.status().nonvolatile.lightScreen = 5;
  return 1;
}

int move_reflect_damage(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& modifier) {
  if (tPKV.status().nonvolatile.reflect > 0) {
    if (mV.getBase().getDamageType() == ATK_PHYSICAL) { modifier *= 0.5; }
  }
  return 1;
}

int move_lightScreen_damage(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& modifier) {
  if (tPKV.status().nonvolatile.lightScreen > 0) {
    if (mV.getBase().getDamageType() == ATK_SPECIAL) { modifier *= 0.5; }
  }
  return 1;
}

int engine_reflect_decrement(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (cPKV.status().nonvolatile.reflect > 0) {
    cPKV.status().nonvolatile.reflect--;
  }

  return 1;
}

int engine_lightScreen_decrement(PkCUEngine& cu, PokemonVolatile cPKV) {
  if (cPKV.status().nonvolatile.lightScreen > 0) {
    cPKV.status().nonvolatile.lightScreen--;
  }

  return 1;
}

void register_move_screens(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "reflect", PLUGIN_ON_EVALUATEMOVE, move_reflect_set, 0, current_team));
  extensions.push_back(plugin(move, "reflect", PLUGIN_ON_BEGINNINGOFTURN, engine_reflect_decrement, -1, current_team));
  extensions.push_back(plugin(move, "reflect", PLUGIN_ON_MODIFYRAWDAMAGE, move_reflect_damage, 0, other_team));
  extensions.push_back(plugin(move, "light screen", PLUGIN_ON_EVALUATEMOVE, move_lightScreen_set, 0, current_team));
  extensions.push_back(plugin(move, "light screen", PLUGIN_ON_BEGINNINGOFTURN, engine_lightScreen_decrement, -1, current_team));
  extensions.push_back(plugin(move, "light screen", PLUGIN_ON_MODIFYRAWDAMAGE, move_lightScreen_damage, 0, other_team));
}

} // namespace gen4
