#include "gen4_scripts_internal.h"

namespace gen4 {

int move_leveledDamage(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Type* resistedType;
  const Move* tMove = &mV.getBase();
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

  tPKV.modHP(-1 * (int)cPKV.nv().getLevel());

  return 1;
};

void register_move_leveled_damage(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "night shade", PLUGIN_ON_EVALUATEMOVE, move_leveledDamage, 0, current_team));
  extensions.push_back(plugin(move, "seismic toss", PLUGIN_ON_EVALUATEMOVE, move_leveledDamage, 0, current_team));
}

} // namespace gen4
