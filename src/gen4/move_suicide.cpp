#include "gen4_scripts_internal.h"

namespace gen4 {

int move_suicide_modLife(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  // suicide occurs regardless of hit or miss. No hasHit check.
  const Move* cMove = &cu.getMV(actor).getBase();
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
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  MoveVolatile mV = cu.getMV(actor);
  if ((&mV.getBase() != explosion_t) && (&mV.getBase() != selfDestruct_t)) {
    return 0;
  }

  modifier *= 2.0;

  return 1;
};

void register_move_suicide(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyAttackPower(move, "explosion", move_suicide_modPower, 0, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "explosion", move_suicide_modLife, 0, current_team));
  extensions.push_back(pluginOnModifyAttackPower(move, "selfdestruct", move_suicide_modPower, 0, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "selfdestruct", move_suicide_modLife, 0, current_team));
  extensions.push_back(pluginOnEndOfMove(move, "memento", move_suicide_modLife, 0, current_team));
}

} // namespace gen4
