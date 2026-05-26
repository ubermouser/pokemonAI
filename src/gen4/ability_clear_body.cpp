#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_restoreStats(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile tPKV = cu.getPKV(target);
  const Move& cMove = cu.getMV(actor).getBase();
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
  extensions.push_back(pluginOnSecondaryEffect(ability, "clear body", ability_restoreStats, 0, other_team));
}

} // namespace gen4
