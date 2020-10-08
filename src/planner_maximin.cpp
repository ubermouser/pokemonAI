#include "../inc/planner_maximin.h"

#include <unordered_map>

PlyResult PlannerMaxiMin::generateSolutionAtDepth(
    const ConstEnvironmentPossible& origin, size_t maxPly) const {
  // a count of the number of nodes evaluated:
  size_t numNodesEvaluated = 0;
  PlyResult result = recurse_alphabeta(
      origin, maxPly, FitnessDepth::worst(), FitnessDepth::best(), &numNodesEvaluated);

  result.numNodes = numNodesEvaluated;

  return result;
};