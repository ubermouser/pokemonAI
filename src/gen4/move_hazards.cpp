#include "gen4_scripts_internal.h"

namespace gen4 {


int move_spikes_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != spikes_t) { return 0; }

  uint32_t initial_spikes = tPKV.status().nonvolatile.spikes;
  tPKV.status().nonvolatile.spikes = std::min(3U, initial_spikes + 1U);

  return 1;
};

int move_toxicSpikes_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != toxicSpikes_t) { return 0; }

  uint32_t initial_toxic = tPKV.status().nonvolatile.toxicSpikes;
  tPKV.status().nonvolatile.toxicSpikes = std::min(2U, initial_toxic + 1U);

  return 1;
}

int move_stealthRock_set(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != stealthRock_t) { return 0; }

  tPKV.status().nonvolatile.stealthRock = 1;

  return 1;
};

int move_spikes_switch(PkCUEngine& cu, PokemonVolatile cPKV) {
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

int move_toxicSpikes_switch(PkCUEngine& cu, PokemonVolatile cPKV) {
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

int move_stealthRock_switch(PkCUEngine& cu, PokemonVolatile cPKV) {
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
  extensions.push_back(plugin(move, "spikes", PLUGIN_ON_SWITCHIN, move_spikes_switch, 2, all_teams));
  extensions.push_back(plugin(move, "spikes", PLUGIN_ON_EVALUATEMOVE, move_spikes_set, 0, current_team));
  extensions.push_back(plugin(move, "toxic spikes", PLUGIN_ON_SWITCHIN, move_toxicSpikes_switch, 2, all_teams));
  extensions.push_back(plugin(move, "toxic spikes", PLUGIN_ON_EVALUATEMOVE, move_toxicSpikes_set, 0, current_team));
  extensions.push_back(plugin(move, "stealth rock", PLUGIN_ON_SWITCHIN, move_stealthRock_switch, 0, all_teams));
  extensions.push_back(plugin(move, "stealth rock", PLUGIN_ON_EVALUATEMOVE, move_stealthRock_set, 0, current_team));
}

} // namespace gen4
