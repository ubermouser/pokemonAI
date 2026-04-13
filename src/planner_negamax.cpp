#include "pokemonai/planner_negamax.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;



po::options_description PlannerNegamax::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc =
      PlannerMaximin::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "ttable-size").c_str(),
      po::value<size_t>(&transposition_table_size)->default_value(defaults.transposition_table_size),
      "size of the transposition table.");

  return desc;
}

PlannerNegamax::PlannerNegamax(const Config& cfg)
    : PlannerMinimax(cfg),
      cfg_(cfg),
      transpositionTable_(cfg_.transposition_table_size),
      orderHeuristic_() {
  resetName();
}

PlannerNegamax& PlannerNegamax::initialize() {
  PlannerMinimax::initialize();

  orderHeuristic_.initialize();
  transpositionTable_.clear();
  return *this;
}


std::vector<ActionMap> PlannerNegamax::getValidActions(
    const ConstEnvironmentPossible& origin,
    TEAM iTeam) const {
  // if this state has been evaluated at a shallower depth, immediately consider the shallower
  //  depth's best move first. Odds are, it will still be quite good.
  ActionMap killerMoveMap;
  if (transpositionTable_.exists(origin.getHash())) {
    auto probe = transpositionTable_.get(origin.getHash());
    killerMoveMap =
        (iTeam == agentTeam_) ? probe.agentAction : probe.otherAction;
  }

  // order the remaining moves as per the butterfly heuristic:
  auto actions = cu_->getAllValidActions(origin.getEnv(), iTeam);
  orderHeuristic_.order(origin.getEnv(), actions, killerMoveMap);

  return actions;
}


bool PlannerNegamax::testAgentSelection(
    EvalResult& bestOfWorst,
    const EvalResult& worst,
    const FitnessDepth& highCutoff,
    const ConstEnvironmentPossible& origin) const {
  bool cutoff =
      base_t::testAgentSelection(bestOfWorst, worst, highCutoff, origin);
  if (cutoff) {
    if (!worst.agentAction.empty()) {
      orderHeuristic_.increment(worst.agentAction);
    }
  }

  return cutoff;
}


bool PlannerNegamax::testOtherSelection(
    EvalResult& worst,
    const EvalResult& current,
    const FitnessDepth& lowCutoff,
    const ConstEnvironmentPossible& origin) const {
  bool cutoff = base_t::testOtherSelection(worst, current, lowCutoff, origin);
  if (cutoff) {
    if (!current.otherAction.empty()) {
      orderHeuristic_.increment(current.otherAction);
    }
  }

  return cutoff;
}


EvalResult PlannerNegamax::recurse_alphabeta(
      const ConstEnvironmentPossible& origin,
      size_t iDepth,
      const FitnessDepth& lowCutoff,
      const FitnessDepth& highCutoff,
      size_t* nodesEvaluated) const {
  EvalResult result;
  bool doRecurse = true;
  if (transpositionTable_.exists(origin.getHash())) {
    result = transpositionTable_.get(origin.getHash());
    // if this result has sufficient minimum depth:
    if (result.depth >= iDepth) {
      // if this result is either 100% evaluated:
      if (result.fitness.fullyEvaluated()) {
        doRecurse = false;
      // else, if this result is evaluated enough to produce a cutoff:
      } else if (testGammaCutoff(result, lowCutoff, highCutoff)) {
        doRecurse = false;
      }
    }
  }

  if (doRecurse) {
    result = PlannerMinimax::recurse_alphabeta(
        origin, iDepth, lowCutoff, highCutoff, nodesEvaluated);
    transpositionTable_.put(origin.getHash(), result);
  }

  return result;
}
