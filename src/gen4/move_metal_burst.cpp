#include "gen4_scripts_internal.h"

namespace gen4 {

int move_metalBurst(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move* cMove = &mV.getBase();
  if (cMove != metalBurst_t) { return 0; }

  // Metal Burst fails if the user moves first
  if (cu.getBase().actor(cu.getICTeam(), ActorProxy::ALL_TEAMMATES).isMovedFirst()) { return 1; }

  // Opponent must have hit with a move
  if (!cu.getBase().actor(cu.getIOTeam(), ActorProxy::ALL_TEAMMATES).isHit()) { return 1; }

  const DamageComponents_t& oDamage =
      cu.getDamageComponent(cu.getIBase(), cu.getIOTeam());

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
  extensions.push_back(plugin(move, "metal burst", PLUGIN_ON_EVALUATEMOVE, move_metalBurst, 0, current_team));
}

} // namespace gen4
