#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int ability_pinch_type_boost(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& basePowerModifier) {
  if (!cPKV.nv().abilityExists()) { return 0; }
  const Ability* ability = &cPKV.nv().getAbility();

  if (cPKV.getPercentHP() > (1.0 / 3.0)) { return 0; }

  const Type* moveType = &mV.getBase().getType();
  const Type* boostedType = nullptr;

  if (ability == blaze_t) {
    boostedType = fire_t;
  } else if (ability == overgrow_t) {
    boostedType = grass_t;
  } else if (ability == swarm_t) {
    boostedType = bug_t;
  } else if (ability == torrent_t) {
    boostedType = water_t;
  } else {
    return 0;
  }

  if (moveType == boostedType) {
    basePowerModifier *= 1.5;
    return 1;
  }

  return 0;
}

void register_ability_pinch_boost(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "blaze", PLUGIN_ON_MODIFYBASEPOWER, ability_pinch_type_boost, -1, current_team));
  extensions.push_back(plugin(ability, "overgrow", PLUGIN_ON_MODIFYBASEPOWER, ability_pinch_type_boost, -1, current_team));
  extensions.push_back(plugin(ability, "swarm", PLUGIN_ON_MODIFYBASEPOWER, ability_pinch_type_boost, -1, current_team));
  extensions.push_back(plugin(ability, "torrent", PLUGIN_ON_MODIFYBASEPOWER, ability_pinch_type_boost, -1, current_team));
}

} // namespace gen4
