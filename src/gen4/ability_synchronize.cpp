#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_synchronize(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  PokemonVolatile tPKV = cu.getPKV(target);
  // Only run if tPKV has Synchronize (guaranteed by registration as other_team)
  if (&tPKV.nv().getAbility() != synchronize_t) { return 0; }

  // Check if tPKV has the status inflicted by the move
  // This plugin runs AFTER the engine (priority -1), so status should be applied by now

  const Move& cMove = cu.getMV(actor).getBase();
  uint32_t targetAilment = cMove.getTargetAilment();

  if (tPKV.getStatusAilment() != targetAilment) { return 0; }

  bool triggersSync = false;
  if (targetAilment == AIL_NV_BURN || targetAilment == AIL_NV_PARALYSIS ||
      targetAilment == AIL_NV_POISON ||
      targetAilment == AIL_NV_POISON_TOXIC) {
    triggersSync = true;
  }

  if (!triggersSync) { return 0; }

  // Apply status to cPKV (Attacker)
  if (cPKV.isAlive() && cPKV.getStatusAilment() == AIL_NV_NONE) {
    // Check immunities?
    // "Synchronize will not affect a Pokémon that is immune to the status condition."
    // Simplified check for now (Type immunity)

    bool immune = false;
    // Fire types immune to Burn
    if (targetAilment == AIL_NV_BURN && cPKV.getBase().hasType(fire_t)) {
      immune = true;
    }
    // Poison/Steel types immune to Poison
    if ((targetAilment == AIL_NV_POISON ||
         targetAilment == AIL_NV_POISON_TOXIC) &&
        (cPKV.getBase().hasType(poison_t) ||
         cPKV.getBase().hasType(steel_t))) {
      immune = true;
    }

    if (!immune) {
      // Toxic becomes regular poison in Gen 4 via Synchronize
      uint32_t syncAilment = targetAilment;
      if (syncAilment == AIL_NV_POISON_TOXIC) {
        syncAilment = AIL_NV_POISON;
      }

      cPKV.status().cTeammate.toxicPoison_tier = 0;
      cPKV.setStatusAilment(syncAilment);
    }
  }

  return 0; // Continue other plugins
}
void register_ability_synchronize(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnSecondaryEffect(ability, "synchronize", ability_synchronize, -1, other_team));
}

} // namespace gen4
