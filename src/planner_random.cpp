//#define PKAI_IMPORT
#include "../inc/planner_random.h"

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"

const std::string PlannerRandom::ident = "Random_Planner-NULLEVAL";


bool PlannerRandom::isInitialized() const {
  if (agentTeam_ >= 2) { return false; }
  if (cu_ == NULL) { return false; }
  
  return true; 
}


uint32_t PlannerRandom::generateSolution(const ConstEnvironmentPossible& origin) {
  // determine the set of all valid actions:
  auto validMoves = cu_->getValidActions(origin, agentTeam_);
  if (validMoves.empty()) {
    return UINT32_MAX;
  } else {
    // choose a completely random action to return:
    size_t iAction = rand() % validMoves.size();
    return validMoves[iAction];
  }
};
