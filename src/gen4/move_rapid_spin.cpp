#include "gen4_scripts_internal.h"

namespace gen4 {

int move_rapidSpin(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  MoveVolatile mV = cu.getMV(actor);
  if (&mV.getBase() != rapidSpin_t) { return 0; }

  // clear trapped:
  if (cPKV.status().cTeammate.trap < 7) { cPKV.status().cTeammate.trap = 0; }
  // clear entry hazards:
  cPKV.status().nonvolatile.toxicSpikes = 0;
  cPKV.status().nonvolatile.spikes = 0;
  cPKV.status().nonvolatile.stealthRock = 0;

  return 1;
};
void register_move_rapid_spin(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfMove(move, "rapid spin", move_rapidSpin, 0, current_team));
}

} // namespace gen4
