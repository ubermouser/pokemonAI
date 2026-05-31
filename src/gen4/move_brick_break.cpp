#include "gen4_scripts_internal.h"

namespace gen4 {

int move_brickBreak_removeScreens(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != brickBreak_t) { return 0; }

  // if the target is immune to fighting type moves, the screens are not removed
  fpType effectiveness = fighting_t->getModifier(tPKV.getBase().getType(0)) *
                         fighting_t->getModifier(tPKV.getBase().getType(1));
  if (effectiveness == 0.0) { return 1; }

  // remove Reflect and Light Screen from the target's team
  TeamVolatile tTV = cu.getTTV();
  tTV.status().reflect = 0;
  tTV.status().lightScreen = 0;

  return 1;
}

void register_move_brick_break(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyRawDamage(move, "brick break", move_brickBreak_removeScreens, -1, current_team));
}

} // namespace gen4
