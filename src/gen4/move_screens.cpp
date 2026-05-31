#include "gen4_scripts_internal.h"

namespace gen4 {

int move_reflect_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != reflect_t) { return 0; }

  TeamVolatile tV = cu.getTV();
  if (tV.status().reflect > 0) { return 1; }

  tV.status().reflect = 5;
  return 1;
}

int move_lightScreen_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != lightScreen_t) { return 0; }

  TeamVolatile tV = cu.getTV();
  if (tV.status().lightScreen > 0) { return 1; }

  tV.status().lightScreen = 5;
  return 1;
}

int move_reflect_damage(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  MoveVolatile mV = cu.getMV(actor);
  TeamVolatile tTV = cu.getTTV();
  if (tTV.status().reflect > 0) {
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
  MoveVolatile mV = cu.getMV(actor);
  TeamVolatile tTV = cu.getTTV();
  if (tTV.status().lightScreen > 0) {
    if (mV.getBase().getDamageType() == ATK_SPECIAL) { modifier *= 0.5; }
  }
  return 1;
}

int engine_reflect_decrement(PkCUEngine& cu, const Actor& actor) {
  TeamVolatile tV = cu.getTV();
  if (actor.iTeammate() == tV.getICPKV() && tV.status().reflect > 0) {
    tV.status().reflect--;
  }

  return 1;
}

int engine_lightScreen_decrement(PkCUEngine& cu, const Actor& actor) {
  TeamVolatile tV = cu.getTV();
  if (actor.iTeammate() == tV.getICPKV() && tV.status().lightScreen > 0) {
    tV.status().lightScreen--;
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
