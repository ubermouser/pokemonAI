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
    const ConstEnvironmentPossible& origin, size_t maxPly) const {
  // a count of the number of nodes evaluated:
  PlyResult result;

  // determine the best action based upon the evaluator's prediction:
  for (const auto& action: cu_->getValidActions(origin, agentTeam_)) {
    FitnessDepth child = recurse_gamma(
        origin, action, Action::wait(), maxPly - 1, result.fitness, Fitness::best(), &result.numNodes);

    // is the returned fitness better than the current best fitness:
    if (child > result) {
      result.fitness = child.fitness;
      result.depth = child.depth;
      result.agentAction = action;
    }
  }

  return result;
};
