//#define PKAI_IMPORT
#include "../inc/planner_random.h"

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"

const std::string PlannerRandom::ident = "Random_Planner-NULLEVAL";


bool PlannerRandom::isInitialized() const 
{ 
  if (agentTeam_ >= 2) { return false; }
  if (cu_ == NULL) { return false; }
  
  return true; 
}


uint32_t PlannerRandom::generateSolution(const ConstEnvironmentPossible& origin)
{
  // choose a completely random action to return:
  size_t iNAction = rand() % AT_ITEM_USE;
  for (size_t _iAction = 0; _iAction != AT_ITEM_USE; ++_iAction) {
    size_t iAction = (iNAction + _iAction) % AT_ITEM_USE;

    if (cu_->isValidAction(origin, iAction, agentTeam_)) { return iAction; }
  }
  // no actions are valid:
  return UINT32_MAX;
};
