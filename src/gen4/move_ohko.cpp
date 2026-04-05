#include "gen4_scripts_internal.h"

namespace gen4 {

int move_ohko_accuracy(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToHit) {
  const Move* cMove = &mV.getBase();
  if (cMove != fissure_t && cMove != guillotine_t && cMove != hornDrill_t &&
      cMove != sheerCold_t) {
    return 0;
  }

  uint32_t userLevel = cPKV.nv().getLevel();
  uint32_t targetLevel = tPKV.nv().getLevel();

  if (userLevel < targetLevel) {
    probabilityToHit = FixType(0.0);
    return 2;
  }

  // Type immunities
  if (cMove == hornDrill_t || cMove == guillotine_t) {
    if (&tPKV.getBase().getType(0) == ghost_t ||
        &tPKV.getBase().getType(1) == ghost_t) {
      if (!tPKV.status().cTeammate.identify) {
        probabilityToHit = FixType(0.0);
        return 2;
      }
    }
  } else if (cMove == fissure_t) {
    bool isImmune = false;
    if (&tPKV.getBase().getType(0) == flying_t ||
        &tPKV.getBase().getType(1) == flying_t) {
      isImmune = true;
    }
    if (tPKV.nv().abilityExists() && (&(tPKV.nv().getAbility()) == levitate_t)) {
      isImmune = true;
    }

    if (isImmune) {
      probabilityToHit = FixType(0.0);
      return 2;
    }
  }

  if (cPKV.status().cTeammate.lockOn) {
    probabilityToHit = FixType(1.0);
  } else {
    probabilityToHit = FixType((float)((userLevel - targetLevel) + 30) / 100.0f);
  }

  return 2;
}

int move_ohko_effect(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move* cMove = &mV.getBase();
  if (cMove != fissure_t && cMove != guillotine_t && cMove != hornDrill_t &&
      cMove != sheerCold_t) {
    return 0;
  }

  // OHKO!
  tPKV.modHP(-1 * (int)tPKV.getHP());

  return 1;
};

void register_move_ohko(
    const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyHitProbability(move, "fissure", move_ohko_accuracy, -5,
      current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "guillotine", move_ohko_accuracy, -5,
      current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "horn drill", move_ohko_accuracy, -5,
      current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "sheer cold", move_ohko_accuracy, -5,
      current_team));

  extensions.push_back(
      pluginOnEvaluateMove(move, "fissure", move_ohko_effect, 0,
             current_team));
  extensions.push_back(
      pluginOnEvaluateMove(move, "guillotine", move_ohko_effect, 0,
             current_team));
  extensions.push_back(
      pluginOnEvaluateMove(move, "horn drill", move_ohko_effect, 0,
             current_team));
  extensions.push_back(
      pluginOnEvaluateMove(move, "sheer cold", move_ohko_effect, 0,
             current_team));
}

} // namespace gen4
