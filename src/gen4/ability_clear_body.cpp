#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_restoreStats(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move& cMove = mV.getBase();
  for (size_t iBuff = 0; iBuff != 9; ++iBuff) {
    uint32_t debuff = cMove.getTargetDebuff(iBuff);
    if (debuff > 0) {
      // restore the stat boost:
      tPKV.modBoost(iBuff, debuff);
    }
  }
  return 1;
};

void register_ability_clear_body(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(ability, "clear body", PLUGIN_ON_SECONDARYEFFECT, ability_restoreStats, 0, other_team));
}

} // namespace gen4
