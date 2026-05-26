#include "gen4_scripts_internal.h"

namespace gen4 {

int move_reflect_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != reflect_t) { return 0; }

  if (cPKV.status().nonvolatile.reflect > 0) { return 1; }

  cPKV.status().nonvolatile.reflect = 5;
  return 1;
}

int move_lightScreen_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != lightScreen_t) { return 0; }

  if (cPKV.status().nonvolatile.lightScreen > 0) { return 1; }

  cPKV.status().nonvolatile.lightScreen = 5;
  return 1;
}

int move_reflect_damage(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (tPKV.status().nonvolatile.reflect > 0) {
    if (mV.getBase().getDamageType() == ATK_PHYSICAL) { modifier *= 0.5; }
  }
  return 1;
}

int move_lightScreen_damage(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (tPKV.status().nonvolatile.lightScreen > 0) {
    if (mV.getBase().getDamageType() == ATK_SPECIAL) { modifier *= 0.5; }
  }
  return 1;
}

int engine_reflect_decrement(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (cPKV.status().nonvolatile.reflect > 0) {
    cPKV.status().nonvolatile.reflect--;
  }

  return 1;
}

int engine_lightScreen_decrement(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (cPKV.status().nonvolatile.lightScreen > 0) {
    cPKV.status().nonvolatile.lightScreen--;
  }

  return 1;
}

void register_move_screens(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "reflect", move_reflect_set, 0, current_team));
  extensions.push_back(pluginOnBeginningOfTurn(engine, "reflect_decrement", engine_reflect_decrement, -1, all_teams));
  extensions.push_back(pluginOnModifyRawDamage(engine, "reflect_damage", move_reflect_damage, 0, all_teams));
  extensions.push_back(pluginOnEvaluateMove(move, "light screen", move_lightScreen_set, 0, current_team));
  extensions.push_back(pluginOnBeginningOfTurn(engine, "light_screen_decrement", engine_lightScreen_decrement, -1, all_teams));
  extensions.push_back(pluginOnModifyRawDamage(engine, "light_screen_damage", move_lightScreen_damage, 0, all_teams));
}

} // namespace gen4
