#include "pokemonai/planner_minimax.h"

bool PlannerMinimax::testGammaCutoff(
    const EvalResult& child,
    const FitnessDepth& lowCutoff,
    const FitnessDepth& highCutoff) const {
  if (child < lowCutoff) {
    // if there's no possibility this action is the best for the agent, do not continue:
    return true;
  } else if (child > highCutoff) {
    // if the other team would never choose this move against the agent, do not continue:
    return true;
  } else if (highCutoff < lowCutoff) {
     // if there exists no solution that can satisfy both the agent and the other, do not continue:
    return true;
  }
  return false;
}


bool PlannerMinimax::testAgentSelection(
    EvalResult& bestOfWorst,
    const EvalResult& worst,
    const FitnessDepth& highCutoff,
    const ConstEnvironmentPossible& origin) const {
  if (worst > bestOfWorst) { bestOfWorst = worst; }

  // is the min of all other agent moves better than the best of our current
  // moves?
  if (bestOfWorst >= highCutoff) { return true; }
  return false;
}


bool PlannerMinimax::testOtherSelection(
    EvalResult& worst,
    const EvalResult& current,
    const FitnessDepth& lowCutoff,
    const ConstEnvironmentPossible& origin) const {
  if (current < worst) { worst = current; }

  // has the other agent improved upon its best score by reducing our score
  // more?
  if (worst <= lowCutoff) { return true; }
  return false;
}
