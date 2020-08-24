#include "../inc/planner_maximin.h"

#include <unordered_map>

PlyResult PlannerMaxiMin::generateSolutionAtDepth(
    const ConstEnvironmentVolatile& origin, size_t maxPly) const {
  // TODO(@drendleman) - replace this loop with recurse_alphabeta
  // a count of the number of nodes evaluated:
  PlyResult result; result.fitness = Fitness::worst();
  std::unordered_map<Action, std::pair<Action, Fitness>> fitnesses;

  // determine the best action based upon the evaluator's prediction:
  for (const auto& actions: cu_->getAllValidActions(origin, agentTeam_)) {
    if (fitnesses.count(actions[0]) == 0) { 
      fitnesses[actions[0]] = {-1, Fitness::best()};
    }
    auto& high = fitnesses[actions[0]];

    // recurse at one depth lower:
    Fitness currentFitness = recurse_gamma(
        origin, actions[0], actions[1], maxPly - 1, result.fitness, high.second, &result.numNodes);

    // has the other agent improved upon its best score by reducing our score more?
    if (currentFitness < high.second) {
      high.first = actions[1];
      high.second = currentFitness;
    }
    // is the returned fitness better than the current best fitness:
    if (high.second > result.fitness) {
      result.agentAction = actions[0];
      result.otherAction = high.first;
      result.fitness = high.second;
    }
  }

  return result;
};