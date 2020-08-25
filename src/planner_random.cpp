//#define PKAI_IMPORT
#include "../inc/planner_random.h"

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"

const std::string PlannerRandom::ident = "Random_Planner-NULLEVAL";


PlyResult PlannerRandom::generateSolutionAtLeaf(
    const ConstEnvironmentVolatile& origin) const {
  PlyResult result;
  // determine if we want to perform moves only:
  bool doMove = (cfg_.moveChance*RAND_MAX) <= (rand() % RAND_MAX);
  // determine the set of all valid actions:
  auto validMoves = cu_->getValidMoveActions(origin, agentTeam_);
  auto validActions = cu_->getValidActions(origin, agentTeam_);
  auto& valid = (doMove && !validMoves.empty())?validMoves:validActions;

  // are there ANY valid actions?
  if (!valid.empty()) {
    // choose a completely random action to return:
    size_t iAction = rand() % valid.size();
    result.agentAction = valid[iAction];
  }

  return result;
};
