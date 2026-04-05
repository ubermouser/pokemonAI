#include "gen4_scripts_internal.h"

namespace gen4 {


int move_highCrit(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    FixType& probabilityToCrit) {
  const Move* tMove = &mV.getBase();
  if ((tMove != airCutter_t) && (tMove != attackOrder_t) &&
      (tMove != blazeKick_t) && (tMove != crabHammer_t) &&
      (tMove != crossChop_t) && (tMove != crossPoison_t) &&
      (tMove != leafBlade_t) && (tMove != nightSlash_t) &&
      (tMove != psychoCut_t) && (tMove != razorLeaf_t) &&
      (tMove != shadowClaw_t) && (tMove != slash_t) && (tMove != stoneEdge_t)) {
    return 0;
  }

  // raise move's crit stage by 1:
  probabilityToCrit = cPKV.getAccuracy_boosted(FV_CRITICALHIT, 1);

  return 1;
}

void register_move_highCrit(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyCritProbability(move, "air cutter", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "attack order", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "blaze kick", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "crabhammer", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "cross chop", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "cross poison", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "leaf blade", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "night slash", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "psycho cut", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "razor leaf", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "shadow claw", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "slash", move_highCrit, -1, current_team));
  extensions.push_back(pluginOnModifyCritProbability(move, "stone edge", move_highCrit, -1, current_team));
}

} // namespace gen4
