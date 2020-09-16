#include "../inc/planner_maximin.h"

#include <unordered_map>

PlyResult PlannerMaxiMin::generateSolutionAtDepth(
    const ConstEnvironmentVolatile& origin, size_t maxPly) const {
  // TODO(@drendleman) - replace this loop with recurse_alphabeta
  // a count of the number of nodes evaluated:
  size_t numNodesEvaluated = 0;
  PlyResult result = recurse_alphabeta(
      origin, maxPly, Fitness::worst(), Fitness::best(), &numNodesEvaluated);

  result.numNodes = numNodesEvaluated;

  return result;
};