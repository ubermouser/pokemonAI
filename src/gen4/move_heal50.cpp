#include "gen4_scripts_internal.h"

namespace gen4 {

int move_heal50(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  const Move* tMove = &cu.getMV(actor).getBase();
  if ((tMove != recover_t) && (tMove != milkDrink_t) && (tMove != slackOff_t) &&
      (tMove != softBoiled_t) && (tMove != healOrder_t) && (tMove != roost_t)) {
    return 0;
  }

  cPKV.modPercentHP(0.50);

  return 1;
};

void register_move_heal50(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "heal order", move_heal50, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "milk drink", move_heal50, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "recover", move_heal50, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "roost", move_heal50, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "slack off", move_heal50, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "softboiled", move_heal50, 0, current_team));
}

} // namespace gen4
