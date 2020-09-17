#include "../inc/planner_minimax.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;



po::options_description PlannerMiniMax::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc = PlannerMaxiMin::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "ttable-size").c_str(),
      po::value<size_t>(&transposition_table_size)->default_value(defaults.transposition_table_size),
      "size of the transposition table.");

  return desc;
}

PlannerMiniMax::PlannerMiniMax(const Config& cfg)
  : PlannerMaxiMin(cfg),
    cfg_(cfg),
    transpositionTable_(cfg_.transposition_table_size),
    orderHeuristic_() {
}

PlannerMiniMax& PlannerMiniMax::initialize() {
  PlannerMaxiMin::initialize();

  orderHeuristic_.initialize();
  transpositionTable_.clear();
  return *this;
}


ActionVector PlannerMiniMax::getValidActions(
    const ConstEnvironmentPossible& origin,
    size_t iTeam) const {
  // TODO(@drendleman) - killer heuristic

  auto actions = cu_->getValidActions(origin, iTeam);
  orderHeuristic_.order(origin, iTeam, actions);

  return actions;
}


bool PlannerMiniMax::testAgentCutoff(
    EvalResult& bestOfWorst,
    const EvalResult& worst,
    const ConstEnvironmentPossible& origin) const {
  bool cutoff = base_t::testAgentCutoff(bestOfWorst, worst, origin);
  if (cutoff) { 
    orderHeuristic_.increment(origin, agentTeam_, worst.agentAction);
  }

  return cutoff;
}


bool PlannerMiniMax::testOtherCutoff(
    EvalResult& worst,
    const EvalResult& current,
    const ConstEnvironmentPossible& origin) const {
  bool cutoff = base_t::testOtherCutoff(worst, current, origin);
  if (cutoff) { 
    orderHeuristic_.increment(origin, otherTeam_, current.otherAction);
  }

  return cutoff;
}


EvalResult PlannerMiniMax::recurse_alphabeta(
      const ConstEnvironmentPossible& origin,
      size_t iDepth,
      const Fitness& lowCutoff,
      const Fitness& highCutoff,
      size_t* nodesEvaluated) const {
  EvalResult result;
  bool doRecurse = true;
  if (transpositionTable_.exists(origin.getHash())) {
    result = transpositionTable_.get(origin.getHash());
    if (result.depth >= iDepth) { 
      doRecurse = false;
      // TODO(@drendleman) - recurse if this was a cutoff node previously but is now a full node
    }
  }

  if (doRecurse) {
    result = base_t::recurse_alphabeta(origin, iDepth, lowCutoff, highCutoff, nodesEvaluated);
    transpositionTable_.put(origin.getHash(), result);
  }

  return result;
}
