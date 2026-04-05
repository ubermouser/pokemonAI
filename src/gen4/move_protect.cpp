#include "gen4_scripts_internal.h"

namespace gen4 {

int move_protect(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != protect_t) { return 0; }

  // Increment counter (capped at 7 to prevent overflow/unnecessary growth)
  uint32_t counter = cPKV.status().cTeammate.protect_counter;
  if (counter < 7) {
    counter++;
  }
  cPKV.status().cTeammate.protect_counter = counter;

  // Calculate success probability: 1 / (2^(counter-1))
  // 1st use (1) -> 100%
  // 2nd use (2) -> 50%
  // 3rd use (3) -> 25%
  FixType probability = FixType(1);
  probability.intValue >>= (counter - 1);

  if (probability < FixType(1)) {
    // Split state: Success vs Failure
    // result[0] = Success (Probability)
    // result[1] = Failure (1 - Probability)
    std::array<size_t, 2> result = {{cu.getIBase(), SIZE_MAX}};
    cu.duplicateState(result, FixType(1) - probability);

    // Success branch
    cu.getPKV(result[0]).status().cTeammate.protected_flag = 1;

    // Failure branch
    cu.getPKV(result[1]).status().cTeammate.protected_flag = 0;
  } else {
    // 100% Success
    cPKV.status().cTeammate.protected_flag = 1;
  }

  return 1;
}

int move_protect_damage(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& damage) {
  // Check if defender is protected
  if (tPKV.status().cTeammate.protected_flag) {
    // Block damage
    cu.getBase().flagsFor(cu.getCActor()).setBlocked();
    damage = 0;
    return 2; // Halt other plugins
  }
  return 0;
}

int move_protect_secondary(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (tPKV.status().cTeammate.protected_flag) {
    return 2; // Block secondary effects
  }
  return 0;
}

int move_protect_status(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (tPKV.status().cTeammate.protected_flag) {
    // Don't block self-targeting moves
    if (&cPKV.nv() == &tPKV.nv()) { return 0; }

    // Block status moves (damageType == 0)
    // Note: Some moves might bypass Protect (e.g. Feint), but ignoring for now as they are exceptions
    if (mV.getBase().getDamageType() == 0) { // ATK_NODMG
      cu.getBase().flagsFor(cu.getCActor()).setBlocked();
      return 2; // Block status move
    }
  }
  return 0;
}

int move_protect_cleanup_end(PkCUEngine& cu, PokemonVolatile cPKV) {
  // Clear protected status at end of round
  cPKV.status().cTeammate.protected_flag = 0;

  // Check if the move used this turn was Protect.
  // If NOT, reset protect_counter.

  bool usedProtect = false;
  Action action = cu.getCAction(); // Action of the pokemon being processed (current team)

  if (action.isMove()) {
    const Move& move = cPKV.nv().getMove_base(action.iMove());
    if (&move == protect_t) {
      usedProtect = true;
    }
  }

  if (!usedProtect) {
    cPKV.status().cTeammate.protect_counter = 0;
  }

  return 0;
}

void register_move_protect(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "protect", PLUGIN_ON_EVALUATEMOVE, move_protect, 0, current_team));
  extensions.push_back(plugin(move, "protect", PLUGIN_ON_CALCULATEDAMAGE, move_protect_damage, -10, all_teams)); // Priority -10 to run early?
  extensions.push_back(plugin(move, "protect", PLUGIN_ON_SECONDARYEFFECT, move_protect_secondary, -10, all_teams));
  extensions.push_back(plugin(engine, "protect_block_status", PLUGIN_ON_EVALUATEMOVE, move_protect_status, -10, all_teams));
  extensions.push_back(plugin(move, "protect", PLUGIN_ON_ENDOFROUND, move_protect_cleanup_end, 0, current_team));
}

} // namespace gen4
