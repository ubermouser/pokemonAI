#include "gen4_scripts_internal.h"

namespace gen4 {

int move_trick(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  PokemonVolatile tPKV = cu.getPKV(target);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != trick_t && &mV.getBase() != switcheroo_t) { return 0; }

  // TODO: Trick fails if the target is behind a substitute.
  if (tPKV.nv().abilityExists()) {
    const auto& ability = tPKV.nv().getAbility();
    if (&ability == stickyHold_t) { return 1; }
  }

  const bool cHasItem = cPKV.hasItem();
  const bool tHasItem = tPKV.hasItem();

  const Item* cItem = cHasItem ? &cPKV.getItem() : nullptr;
  const Item* tItem = tHasItem ? &tPKV.getItem() : nullptr;

  if (tHasItem) {
    cPKV.setItem(*tItem);
  } else {
    cPKV.setNoItem();
  }

  if (cHasItem) {
    tPKV.setItem(*cItem);
  } else {
    tPKV.setNoItem();
  }

  return 1;
};

void register_move_trick(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "trick", move_trick, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "switcheroo", move_trick, 0, current_team));
}

} // namespace gen4
