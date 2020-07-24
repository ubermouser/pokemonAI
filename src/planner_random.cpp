//#define PKAI_IMPORT
#include "../inc/planner_random.h"

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"

const std::string planner_random::ident = "Random_Planner-NULLEVAL";

planner_random::planner_random()
  : cu(NULL),
  agentTeam(SIZE_MAX)
{
};

planner_random::planner_random(const planner_random& other)
  : cu(other.cu),
  agentTeam(other.agentTeam)
{
};

void planner_random::setEnvironment(pkCU& _cu, size_t _agentTeam)
{
  cu = &_cu;
  agentTeam = _agentTeam;
};

bool planner_random::isInitialized() const 
{ 
  if (agentTeam >= 2) { return false; }
  if (cu == NULL) { return false; }
  
  return true; 
}

uint32_t planner_random::generateSolution(const environment_possible& origin)
{
  // choose a completely random action to return:
  size_t iNAction = rand() % AT_ITEM_USE;
  for (size_t _iAction = 0; _iAction != AT_ITEM_USE; ++_iAction)
  {
    size_t iAction = (iNAction + _iAction) % AT_ITEM_USE;

    if (cu->isValidAction(origin.getEnv(), iAction, agentTeam)) { return iAction; }
  }
  // no actions are valid:
  return UINT32_MAX;
};
