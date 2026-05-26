#include "gen4_scripts_internal.h"

namespace gen4 {

int move_alwaysHits(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    FixType& probabilityToHit) {
  const Move* cMove = &cu.getMV(actor).getBase();
  if ((cMove != auraSphere_t) && (cMove != shockWave_t) &&
      (cMove != magnetBomb_t) && (cMove != shadowPunch_t) &&
      (cMove != magicalLeaf_t) && (cMove != aerialAce_t) &&
      (cMove != faintAttack_t) && (cMove != swift_t) && (cMove != struggle_t)) {
    return 0;
  }

  probabilityToHit = FixType(1.0f);

  // do not allow anything to affect hit chance other than this if the move
  // always hits:
  return 2;
}

void register_move_alwaysHits(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyHitProbability(move, "aerial ace", move_alwaysHits, -1, current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "aura sphere", move_alwaysHits, -1, current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "faint attack", move_alwaysHits, -1, current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "magical leaf", move_alwaysHits, -1, current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "magnet bomb", move_alwaysHits, -1, current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "shadow punch", move_alwaysHits, -1, current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "shock wave", move_alwaysHits, -1, current_team));
  extensions.push_back(pluginOnModifyHitProbability(move, "swift", move_alwaysHits, -1, current_team));

  extensions.push_back(pluginOnModifyHitProbability(engine, "struggle always hits effect", move_alwaysHits, -1, all_teams));
}

} // namespace gen4
