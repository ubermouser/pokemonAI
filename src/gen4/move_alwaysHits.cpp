#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int move_alwaysHits(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToHit) {
  const Move* cMove = &mV.getBase();
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
  extensions.push_back(plugin(move, "aerial ace", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, current_team));
  extensions.push_back(plugin(move, "aura sphere", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, current_team));
  extensions.push_back(plugin(move, "faint attack", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, current_team));
  extensions.push_back(plugin(move, "magical leaf", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, current_team));
  extensions.push_back(plugin(move, "magnet bomb", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, current_team));
  extensions.push_back(plugin(move, "shadow punch", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, current_team));
  extensions.push_back(plugin(move, "shock wave", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, current_team));
  extensions.push_back(plugin(move, "swift", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, current_team));

  extensions.push_back(plugin(engine, "struggle always hits effect", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, all_teams));
}

} // namespace gen4
