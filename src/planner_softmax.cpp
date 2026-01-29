#include "pokemonai/planner_softmax.h"

#include <cmath>
#include <numeric>
#include <vector>
#include <fmt/format.h>
#include <boost/program_options.hpp>

#include "pokemonai/roulette.h"

namespace po = boost::program_options;

po::options_description PlannerSoftmax::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc = base_t::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "temperature").c_str(),
      po::value<double>(&temperature)->default_value(defaults.temperature),
      "Temperature parameter for softmax action selection. "
      "Low temperature (T->0) is greedy, high (T->inf) is random.");

  return desc;
}

void PlannerSoftmax::resetName() {
  std::string evalName =
      ((eval_ != NULL) ? fmt::format("-{}", eval_->getName()) : "");
  setName(fmt::format("{}(d={},T={:3.2f}){}", baseName(), cfg_.maxDepth, cfg_.temperature, evalName));
}

PlyResult PlannerSoftmax::generateSolutionAtDepth(
    const ConstEnvironmentPossible& origin, size_t maxPly) const {
  auto agentActions = getValidActions(origin, agentTeam_);
  if (agentActions.empty()) {
    return PlyResult{};
  }

  // If temperature is 0, fall back to MaxiMin deterministic behavior
  if (cfg_.temperature <= 0.0) {
    return base_t::generateSolutionAtDepth(origin, maxPly);
  }

  std::vector<EvalResult> actionValues;
  actionValues.reserve(agentActions.size());

  size_t totalNodes = 0;

  // For each agent action, find the worst-case (negamax) outcome
  for (const auto& agentAction : agentActions) {
    size_t nodesEvaluated = 0;
    EvalResult worst = recurse_beta(
        origin,
        agentAction,
        maxPly,
        FitnessDepth::worst(),
        FitnessDepth::best(),
        &nodesEvaluated);
    totalNodes += nodesEvaluated;
    actionValues.push_back(worst);
  }

  // Select action using Boltzmann distribution
  size_t selectedIdx = selectAction(actionValues);
  
  PlyResult result = actionValues[selectedIdx];
  result.numNodes = totalNodes;

  return result;
}


size_t PlannerSoftmax::selectAction(const std::vector<EvalResult>& actionValues) const {
  // Calculate softmax weights: w_i = exp(Q_i / T)
  // We use max subtraction for numerical stability
  double maxFitness = -INFINITY;
  for (const auto& res : actionValues) {
    maxFitness = std::max(maxFitness, (double)res.fitness.lowerBound());
  }

  std::vector<double> weights;
  weights.reserve(actionValues.size());
  for (const auto& res : actionValues) {
    double q = res.fitness.lowerBound();
    weights.push_back(std::exp((q - maxFitness) / cfg_.temperature));
  }

  // Select action using roulette wheel (Boltzmann distribution)
  return roulette<double>::select(weights);
}
