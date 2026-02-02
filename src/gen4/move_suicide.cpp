#include "gen4_scripts_internal.h"

namespace gen4 {

int move_suicide_modLife(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  // suicide occurs regardless of hit or miss. No hasHit check.

  const Move* cMove = &mV.getBase();
  if ((cMove != explosion_t) && (cMove != selfDestruct_t) &&
      (cMove != memento_t)) {
    return 0;
  }

  // kill pokemon:
  cPKV.setHP(0);

  // always return 2, because we killed the pokemon
  return 2;
};

int move_suicide_modPower(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& modifier) {
  if ((&mV.getBase() != explosion_t) && (&mV.getBase() != selfDestruct_t)) {
    return 0;
  }

  modifier *= 2.0;

  return 1;
};

void register_move_suicide(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "explosion", PLUGIN_ON_MODIFYATTACKPOWER, move_suicide_modPower, 0, current_team));
  extensions.push_back(plugin(move, "explosion", PLUGIN_ON_ENDOFMOVE, move_suicide_modLife, 0, current_team));
  extensions.push_back(plugin(move, "selfdestruct", PLUGIN_ON_MODIFYATTACKPOWER, move_suicide_modPower, 0, current_team));
  extensions.push_back(plugin(move, "selfdestruct", PLUGIN_ON_ENDOFMOVE, move_suicide_modLife, 0, current_team));
  extensions.push_back(plugin(move, "memento", PLUGIN_ON_ENDOFMOVE, move_suicide_modLife, 0, current_team));
}

} // namespace gen4
