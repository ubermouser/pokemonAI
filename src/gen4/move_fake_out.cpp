#include "gen4_scripts_internal.h"

namespace gen4 {

int move_fakeOut_preempt(PkCUEngine& cu, const Actor& actor) {
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != fakeOut_t) { return 0; }

  PokemonVolatile cPKV = cu.getPKV(actor);
  if (cPKV.status().numRoundsInPlay > 0) {
    cu.getBase().flagsFor(actor).setBlocked();
  }
  return 1;
}

void register_move_fake_out(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnBeginningOfTurn(move, "fake out", move_fakeOut_preempt, 0, current_team));
}

} // namespace gen4
