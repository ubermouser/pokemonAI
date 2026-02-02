#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int move_heal50(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  const Move* tMove = &mV.getBase();
  if ((tMove != recover_t) && (tMove != milkDrink_t) && (tMove != slackOff_t) &&
      (tMove != softBoiled_t) && (tMove != healOrder_t) && (tMove != roost_t)) {
    return 0;
  }

  cPKV.modPercentHP(0.50);

  return 1;
};

void register_move_heal50(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "heal order", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, current_team));
  extensions.push_back(plugin(move, "milk drink", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, current_team));
  extensions.push_back(plugin(move, "recover", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, current_team));
  extensions.push_back(plugin(move, "roost", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, current_team));
  extensions.push_back(plugin(move, "slack off", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, current_team));
  extensions.push_back(plugin(move, "softboiled", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, current_team));
}

} // namespace gen4
