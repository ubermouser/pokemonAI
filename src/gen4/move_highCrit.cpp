#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

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
  extensions.push_back(plugin(move, "air cutter", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "attack order", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "blaze kick", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "crabhammer", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "cross chop", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "cross poison", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "leaf blade", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "night slash", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "psycho cut", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "razor leaf", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "shadow claw", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "slash", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
  extensions.push_back(plugin(move, "stone edge", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, current_team));
}

} // namespace gen4
