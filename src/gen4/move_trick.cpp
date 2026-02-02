#include "gen4_scripts_internal.h"

namespace gen4 {

int move_trick(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  if (&mV.getBase() != trick_t) { return 0; }

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
  extensions.push_back(plugin(move, "trick", PLUGIN_ON_EVALUATEMOVE, move_trick, 0, current_team));
}

} // namespace gen4
