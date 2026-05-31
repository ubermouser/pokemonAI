#include "gen4_scripts_internal.h"

namespace gen4 {

int move_leveledDamage(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  PokemonVolatile tPKV = cu.getPKV(target);
  const Type* resistedType;
  const Move* tMove = &cu.getMV(actor).getBase();
  if (tMove == seismicToss_t) {
    resistedType = ghost_t;
  } else if (tMove == nightShade_t) {
    resistedType = normal_t;
  } else {
    return 0;
  }

  // no damage if pokemon's class is of the resited type:
  const PokemonBase& tPKB = tPKV.getBase();
  if ((&tPKB.getType(0) == resistedType) ||
      (&tPKB.getType(1) == resistedType)) {
    return 1;
  }

  uint32_t damage = cPKV.nv().getLevel();
  // SPECIAL CASE: leveled damage against substitute
  if (tPKV.status().substitute > 0 && tPKV.status().substitute != 255) {
    uint32_t subHP = tPKV.status().substitute;
    if (damage >= subHP) {
      tPKV.status().substitute = 255;  // BROKEN_SUB_FLAG
    } else {
      tPKV.status().substitute -= damage;
    }
  } else {
    tPKV.modHP(-1 * (int)damage);
  }

  return 1;
};

void register_move_leveled_damage(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "night shade", move_leveledDamage, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "seismic toss", move_leveledDamage, 0, current_team));
}

} // namespace gen4
