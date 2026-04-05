#include "gen4_scripts_internal.h"

namespace gen4 {


int move_counterMirrorCoat(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move* cMove = &mV.getBase();
  if (cMove != counter_t && cMove != mirrorCoat_t) { return 0; }

  // Must move second
  if (cu.getBase().flagsFor(cu.getICTeam(), ActorProxy::ALL_TEAMMATES).isMovedFirst()) { return 1; }

  // Opponent must have hit with a move
  if (!cu.getBase().flagsFor(cu.getIOTeam(), ActorProxy::ALL_TEAMMATES).isHit()) { return 1; }

  const DamageComponents_t& oDamage =
      cu.getDamageComponent(cu.getIBase(), cu.getIOTeam());
  uint32_t oCategory = oDamage.category;

  if (cMove == counter_t) {
    if (oCategory != ATK_PHYSICAL) { return 1; }
    // Ghost type immunity
    if (tPKV.getBase().hasType(ghost_t)) { return 1; }
  } else {
    assert(cMove == mirrorCoat_t);
    if (oCategory != ATK_SPECIAL) { return 1; }
    // Dark type immunity
    if (tPKV.getBase().hasType(dark_t)) { return 1; }
  }

  uint32_t damageTaken = oDamage.damage;
  if (damageTaken == 0) { return 1; }

  uint32_t damageToDeal = damageTaken * 2;

  // Apply damage
  tPKV.modHP(-(int32_t)damageToDeal);

  return 1;
};

void register_move_counter_mirror_coat(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "counter", PLUGIN_ON_EVALUATEMOVE, move_counterMirrorCoat, 0, current_team));
  extensions.push_back(plugin(move, "mirror coat", PLUGIN_ON_EVALUATEMOVE, move_counterMirrorCoat, 0, current_team));
}

} // namespace gen4
