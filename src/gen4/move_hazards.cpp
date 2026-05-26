#include "gen4_scripts_internal.h"

namespace gen4 {


int move_spikes_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != spikes_t) { return 0; }

  uint32_t initial_spikes = tPKV.status().nonvolatile.spikes;
  tPKV.status().nonvolatile.spikes = std::min(3U, initial_spikes + 1U);

  return 1;
};

int move_toxicSpikes_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != toxicSpikes_t) { return 0; }

  uint32_t initial_toxic = tPKV.status().nonvolatile.toxicSpikes;
  tPKV.status().nonvolatile.toxicSpikes = std::min(2U, initial_toxic + 1U);

  return 1;
}

int move_stealthRock_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != stealthRock_t) { return 0; }

  tPKV.status().nonvolatile.stealthRock = 1;

  return 1;
};

int move_spikes_switch(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  // spikes deals no damage if the pokemon is flying type:
  if (cPKV.getBase().hasType(flying_t)) { return 0; }

  switch (cPKV.status().nonvolatile.spikes) {
  case 3:  // deal damage based on tier:
    cPKV.modPercentHP(-0.25);
    return 1;
  case 2:
    cPKV.modPercentHP(-0.1875);
    return 1;
  case 1:
    cPKV.modPercentHP(-0.125);
    return 1;
  default:
  case 0:
    return 0;
  }
};

int move_toxicSpikes_switch(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (cPKV.getStatusAilment() != AIL_NV_NONE) { return 0; }
  switch (cPKV.status().nonvolatile.toxicSpikes) {
  case 2:  // inflict a type of poison based on tier:
    cPKV.setStatusAilment(AIL_NV_POISON_TOXIC);
    return 1;
  case 1:
    cPKV.setStatusAilment(AIL_NV_POISON);
    return 1;
  default:
  case 0:
    return 0;
  }
};

int move_stealthRock_switch(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (cPKV.status().nonvolatile.stealthRock > 0) {
    // deal damage:
    fpType damage =
        -0.125 *                                          // base damage
        rock_t->getModifier(cPKV.getBase().getType(0)) *  // resistance to rock
        rock_t->getModifier(cPKV.getBase().getType(1));

    cPKV.modPercentHP(damage);

    return 1;
  }

  return 0;
};

void register_move_hazards(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  // clang-format off
  extensions.push_back(pluginOnSwitchIn(engine, "spikes switch", move_spikes_switch, 2, all_teams));
  extensions.push_back(pluginOnEvaluateMove(move, "spikes", move_spikes_set, 0, current_team));
  extensions.push_back(pluginOnSwitchIn(engine, "toxic spikes switch", move_toxicSpikes_switch, 2, all_teams));
  extensions.push_back(pluginOnEvaluateMove(move, "toxic spikes", move_toxicSpikes_set, 0, current_team));
  extensions.push_back(pluginOnSwitchIn(engine, "stealth rock switch", move_stealthRock_switch, 0, all_teams));
  extensions.push_back(pluginOnEvaluateMove(move, "stealth rock", move_stealthRock_set, 0, current_team));
  // clang-format on
}

} // namespace gen4
