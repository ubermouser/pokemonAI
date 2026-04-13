//#define PKAI_IMPORT
#include "pokemonai/planner_max.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <boost/foreach.hpp>

#include "pokemonai/evaluator.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/fp_compare.h"
#include "pokemonai/fitness.h"

#include "pokemonai/environment_possible.h"
#include "pokemonai/environment_nonvolatile.h"


PlannerMax& PlannerMax::setEngine(const std::shared_ptr<PkCU>& cu) {
  // copy, rather than share
  Planner::setEngine(std::make_shared<PkCU>(*cu));
  cu_->setAllowInvalidMoves(PkCU::ActionValidationMethod::WAIT_ONLY);
  return *this;
}


PlyResult PlannerMax::generateSolutionAtDepth(
    const ConstEnvironmentPossible& origin, size_t maxPly) const {
  // a count of the number of nodes evaluated:
  PlyResult result;

  TEAM otherTeam = (agentTeam_ == TEAM_A) ? TEAM_B : TEAM_A;
  const auto& teamV = origin.getEnv().getTeam(otherTeam);
  ActionMap otherAction = getBaselineActionMap(teamV);

  // determine the best action based upon the evaluator's prediction:
  for (const auto& action :
       cu_->getAllValidActions(origin.getEnv(), agentTeam_)) {
    EvalResult child = recurse_gamma(
        origin,
        action,
        otherAction,
        maxPly - 1,
        result.fitness,
        FitnessDepth::best(),
        &result.numNodes);

    // is the returned fitness better than the current best fitness:
    if (child > result) {
      result.fitness = child.fitness;
      result.depth = child.depth;
      result.agentAction = action;
    }
  }

  return result;
};
