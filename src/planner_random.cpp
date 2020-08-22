//#define PKAI_IMPORT
#include "../inc/planner_random.h"

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"

const std::string PlannerRandom::ident = "Random_Planner-NULLEVAL";


PlyResult PlannerRandom::generateSolutionAtDepth(
    const ConstEnvironmentPossible& origin, size_t maxPly) const {
  PlyResult result;
  // determine the set of all valid actions:
  auto validMoves = cu_->getValidActions(origin, agentTeam_);
  if (!validMoves.empty()) {
    // choose a completely random action to return:
    size_t iAction = rand() % validMoves.size();
    result.agentAction = validMoves[iAction];
  }

  return result;
};
