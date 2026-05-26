#include "gen4_scripts_internal.h"

namespace gen4 {

int move_metalBurst(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile tPKV = cu.getPKV(target);
  const Move* cMove = &cu.getMV(actor).getBase();
  if (cMove != metalBurst_t) { return 0; }

  // Metal Burst fails if the user moves first
  if (cu.getBase().flagsFor((TEAM)cu.getICTeam()).isMovedFirst()) { return 1; }

  // TODO - these automatically target the damage dealer, they aren't targeted
  // Opponent must have hit with a move
  if (!cu.getBase().flagsFor((TEAM)cu.getIOTeam()).isHit()) { return 1; }

  const DamageComponents_t& oDamage =
      cu.getDamageComponent(cu.getIBase(), cu.getTarget());

  // Check if damage was taken
  uint32_t damageTaken = oDamage.damage;
  if (damageTaken == 0) { return 1; }

  // Metal Burst deals 1.5x damage
  uint32_t damageToDeal = damageTaken * 1.5;

  // Apply damage
  tPKV.modHP(-(int32_t)damageToDeal);

  return 1;
};

void register_move_metal_burst(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "metal burst", move_metalBurst, 0, current_team));
}

} // namespace gen4
