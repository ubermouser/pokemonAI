#include "gen4_scripts_internal.h"

namespace gen4 {

int move_suckerPunch_noDamageOnCondition(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& raw_damage) {
  if (&mV.getBase() != suckerPunch_t) { return 0; }

  // if the enemy's move is NOT a damaging move:
  auto damageType = cu.getOAction().isMove()
                        ? cu.getTMV().getBase().getDamageType()
                        : ATK_NODMG;
  bool enemyDamagingAction =
      damageType == ATK_PHYSICAL || damageType == ATK_SPECIAL;
  // if the enemy moves first:
  bool enemyMovedFirst =
      cu.getBase().flagsFor((TEAM)cu.getIOTeam()).isMovedFirst();
  if (!enemyDamagingAction || enemyMovedFirst) {
    // the move does not deal damage if these conditions are met:
    raw_damage = 0;
  }

  return 1;
}

void register_move_sucker_punch(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnCalculateDamage(move, "sucker punch", move_suckerPunch_noDamageOnCondition, 0, current_team));
}

} // namespace gen4
