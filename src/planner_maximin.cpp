#include "../inc/planner_maximin.h"

#include <unordered_map>

PlyResult PlannerMaxiMin::generateSolutionAtDepth(
    const ConstEnvironmentPossible& origin, size_t maxPly) const {
  // a count of the number of nodes evaluated:
  PlyResult result;
  Fitness& lowCutoff = result.fitness;
  Action& bestAgentAction = result.agentAction;
  Action& bestOtherAction = result.otherAction;
  std::unordered_map<Action, std::pair<Action, Fitness>> fitnesses;

  // determine the best action based upon the evaluator's prediction:
  for (const auto& actions: cu_->getAllValidActions(origin, agentTeam_)) {
    if (fitnesses.count(actions[0]) == 0) { 
      fitnesses[actions[0]] = {-1, Fitness::best()};
    }
    auto& high = fitnesses[actions[0]];

    Fitness currentFitness = evaluateLeaf(
        origin, actions[0], actions[1], lowCutoff, high.second, &result.numNodes);

    // has the other agent improved upon its best score by reducing our score more?
    if (currentFitness <= high.second) {
      high.first = actions[1];
      high.second = currentFitness;
    }
    // is the returned fitness better than the current best fitness:
    if (high.second >= lowCutoff) {
      bestAgentAction = actions[0];
      bestOtherAction = high.first;
      lowCutoff = high.second;
    }
  }

  return result;
};