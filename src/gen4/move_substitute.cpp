#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

static const uint32_t BROKEN_SUB_FLAG = 255;

int move_substitute(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != substitute_t) { return 0; }

  // Fails if a substitute already exists
  if (cPKV.status().cTeammate.substitute > 0) { return 1; }

  uint32_t cost = cPKV.nv().getMaxHP() / 4;
  // Fails if HP is not enough to create a substitute
  if (cPKV.getHP() <= cost) { return 1; }

  if (cost >= 255) { cost = 254; }

  cPKV.modHP(-(int32_t)cost);
  cPKV.status().cTeammate.substitute = cost;

  return 1;
};

int move_substitute_damage(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& raw_damage) {
  // Check for flag or 0
  if (tPKV.status().cTeammate.substitute == 0 || tPKV.status().cTeammate.substitute == BROKEN_SUB_FLAG) { return 0; }

  uint32_t subHP = tPKV.status().cTeammate.substitute;
  if (raw_damage >= subHP) {
    tPKV.status().cTeammate.substitute = BROKEN_SUB_FLAG;
  } else {
    tPKV.status().cTeammate.substitute -= raw_damage;
  }

  raw_damage = 0;
  return 2;  // Halts other damage plugins
};

int move_substitute_block_secondary(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // substitute > 0 includes BROKEN_SUB_FLAG (255)
  if (tPKV.status().cTeammate.substitute > 0) {
    // Does not block self-targeting secondary effects
    if (&cPKV.nv() == &tPKV.nv()) { return 0; }

    return 2;
  }
  return 0;
};

int move_substitute_block_status(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (tPKV.status().cTeammate.substitute > 0) {
    if (mV.getBase().getDamageType() == ATK_NODMG) {
      // Gen 4 exceptions: Taunt bypasses Substitute
      if (&mV.getBase() == taunt_t) { return 0; }

      // Does not block self-targeting moves
      if (&cPKV.nv() == &tPKV.nv()) { return 0; }

      return 2;  // Consume evaluation, effective failure
    }
  }
  return 0;
};

int move_substitute_cleanup_preturn(PkCUEngine& cu, Action& action) {
  PokemonVolatile cPKV = cu.getPKV();
  if (cPKV.status().cTeammate.substitute == BROKEN_SUB_FLAG) {
    cPKV.status().cTeammate.substitute = 0;
  }
  return 1;
}

int move_substitute_cleanup_end(PkCUEngine& cu, PokemonVolatile cPKV) {
  // cPKV is the attacker.
  // We want to clean up the defender (tPKV) if they have a broken substitute.
  // Also checking cPKV just in case.
  if (cPKV.isAlive() && (cPKV.status().cTeammate.substitute == BROKEN_SUB_FLAG)) {
    cPKV.status().cTeammate.substitute = 0;
  }

  PokemonVolatile tPKV = cu.getTPKV();
  if (tPKV.isAlive() && (tPKV.status().cTeammate.substitute == BROKEN_SUB_FLAG)) {
    tPKV.status().cTeammate.substitute = 0;
  }
  return 1;
}

void register_move_substitute(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "substitute", PLUGIN_ON_EVALUATEMOVE, move_substitute, 0, current_team));
  extensions.push_back(plugin(move, "substitute", PLUGIN_ON_CALCULATEDAMAGE, move_substitute_damage, 0, all_teams));
  extensions.push_back(plugin(move, "substitute", PLUGIN_ON_SECONDARYEFFECT, move_substitute_block_secondary, -10, all_teams));
  extensions.push_back(plugin(move, "substitute", PLUGIN_ON_MODIFYACTION, move_substitute_cleanup_preturn, 0, current_team));
  extensions.push_back(plugin(move, "substitute", PLUGIN_ON_ENDOFTURN, move_substitute_cleanup_end, 0, all_teams));

  extensions.push_back(plugin(engine, "substitute_block_status", PLUGIN_ON_EVALUATEMOVE, move_substitute_block_status, -10, all_teams));   // TODO - conflicts with move_substitute!
}

} // namespace gen4
