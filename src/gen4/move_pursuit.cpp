#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int move_pursuit_modBracket(
    PkCUEngine& cu, MoveVolatile mV, PokemonVolatile cPKV, int32_t& bracket) {
  if (&mV.getBase() != pursuit_t) { return 0; }

  // if the enemy's move is a swap move:
  if (!cu.getOAction().isSwitch()) { return 1; }

  // increase the speed bracket such that it outspeeds a switch-in:
  bracket = 7;
  return 1;
}

int move_pursuit_modPower(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& modifier) {
  if (&mV.getBase() != pursuit_t) { return 0; }

  // if the enemy's move is a swap move:
  if (!cu.getOAction().isSwitch()) { return 1; }

  // greatly increase the power of the move if the enemy's move is a switch-in
  modifier *= 2.0;
  return 1;
}

int move_pursuit_modAccuracy(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToHit) {
  if (&mV.getBase() != pursuit_t) { return 0; }

  // if the enemy's move is a swap move:
  if (!cu.getOAction().isSwitch()) { return 1; }

  // the move never misses if the enemy move is a switch-in:
  probabilityToHit = FixType(1.0f);
  return 2;
}

void register_move_pursuit(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "pursuit", PLUGIN_ON_SETSPEEDBRACKET, move_pursuit_modBracket, 0, current_team));
  extensions.push_back(plugin(move, "pursuit", PLUGIN_ON_MODIFYHITPROBABILITY, move_pursuit_modAccuracy, 0, current_team));
  extensions.push_back(plugin(move, "pursuit", PLUGIN_ON_MODIFYRAWDAMAGE, move_pursuit_modPower, 0, current_team));
}

} // namespace gen4
