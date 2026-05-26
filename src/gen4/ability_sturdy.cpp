#include "gen4_scripts_internal.h"

namespace gen4 {

int ability_sturdy(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    FixType& probabilityToHit) {
  const Move* cMove = &cu.getMV(actor).getBase();
  if (cMove == fissure_t || cMove == guillotine_t || cMove == hornDrill_t ||
      cMove == sheerCold_t) {
    probabilityToHit = FixType(0.0);
    return 2;
  }

  return 0;
};

void register_ability_sturdy(
    const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyHitProbability(ability, "sturdy", ability_sturdy, -10,
      other_team));
}

} // namespace gen4
