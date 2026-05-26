#include "gen4_scripts_internal.h"

namespace gen4 {

int move_cureNonVolatile_team(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  const Move* tMove = &cu.getMV(actor).getBase();
  if ((tMove != aromatherapy_t) && (tMove != healBell_t)) { return 0; }

  // clear nonvolatile:
  TeamVolatile cTMV = cu.getTV();
  switch (cTMV.nv().getNumTeammates()) {
  case 6:
    cTMV.teammate(5).clearStatusAilment();
  case 5:
    cTMV.teammate(4).clearStatusAilment();
  case 4:
    cTMV.teammate(3).clearStatusAilment();
  case 3:
    cTMV.teammate(2).clearStatusAilment();
  case 2:
    cTMV.teammate(1).clearStatusAilment();
  case 1:
  default:
    cTMV.teammate(0).clearStatusAilment();
  };

  // clear volatile confusion:
  cTMV.status().cTeammate.confused = 0;

  return 1;
};

void register_move_cure_team(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEvaluateMove(move, "aromatherapy", move_cureNonVolatile_team, 0, current_team));
  extensions.push_back(pluginOnEvaluateMove(move, "heal bell", move_cureNonVolatile_team, 0, current_team));
}

} // namespace gen4
