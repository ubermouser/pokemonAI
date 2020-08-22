//#define PKAI_IMPORT
#include "../inc/planner_max.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <boost/foreach.hpp>

#include "../inc/evaluator.h"
#include "../inc/pkCU.h"
#include "../inc/fp_compare.h"
#include "../inc/fitness.h"

#include "../inc/environment_possible.h"
#include "../inc/environment_nonvolatile.h"


PlannerMax& PlannerMax::setEngine(const std::shared_ptr<PkCU>& cu) {
  // copy, rather than share
  Planner::setEngine(std::make_shared<PkCU>(*cu));
  cu_->setAllowInvalidMoves();
  return *this;
}


PlyResult PlannerMax::generateSolutionAtDepth(
    const ConstEnvironmentVolatile& origin, size_t maxPly) const {
  // a count of the number of nodes evaluated:
  PlyResult result;

  // TODO(@drendleman) in the case of a tie, the agent should always bias towards a damaging action
  // determine the best action based upon the evaluator's prediction:
  for (const auto& action: cu_->getValidActions(origin, agentTeam_)) {
    Fitness currentFitness = evaluateLeaf(
        origin, action, AT_MOVE_NOTHING, result.fitness, &result.numNodes);

    // is the returned fitness better than the current best fitness:
    if (currentFitness > result.fitness) {
      result.fitness = currentFitness;
      result.agentAction = action;
    }
  }
  
  return result;
};
