#include "gen4_scripts_internal.h"

namespace gen4 {

int move_leechSeed_set(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != leechSeed_t) { return 0; }

  // Fails against Grass types
  if (tPKV.getBase().hasType(grass_t)) { return 0; }

  // Fails if already seeded
  if (tPKV.status().leechSeed) { return 0; }

  tPKV.status().leechSeed = 1;

  return 1;
}

int move_leechSeed_effect(PkCUEngine& cu, const Actor& actor) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (!cPKV.status().leechSeed) { return 0; }

  // Leech Seed deals 1/8 max HP damage
  uint32_t initialHP = cPKV.getHP();
  cPKV.modPercentHP(-0.125);
  uint32_t finalHP = cPKV.getHP();
  int32_t hpDrained = (int32_t)initialHP - (int32_t)finalHP;

  // TODO: heal the actor who cast leech seed
  // Heal the opponent
  PokemonVolatile tPKV = cu.getTTV().getPKV();
  if (tPKV.isAlive()) {
    tPKV.modHP(hpDrained);
  }

  return (cPKV.isAlive() ? 1 : 2);
};

void register_move_leech_seed(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "leech seed", move_leechSeed_set, 0, current_team));
  extensions.push_back(pluginOnEndOfRound(engine, "leech_seed_effect", move_leechSeed_effect, 0, all_teams));
}

} // namespace gen4
