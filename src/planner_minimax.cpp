#include "pokemonai/planner_minimax.h"

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
  resetName();
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
  // if this state has been evaluated at a shallower depth, immediately consider the shallower
  //  depth's best move first. Odds are, it will still be quite good.
  Action killerMove;
  if (transpositionTable_.exists(origin.getHash())) {
    auto probe = transpositionTable_.get(origin.getHash());
    killerMove = (iTeam==agentTeam_)?probe.agentAction:probe.otherAction;
  }

  // order the remaining moves as per the butterfly heuristic:
  auto actions = cu_->getValidActions(origin, iTeam);
  orderHeuristic_.order(origin, iTeam, actions, killerMove);

  return actions;
}


bool PlannerMiniMax::testAgentSelection(
    EvalResult& bestOfWorst,
    const EvalResult& worst,
    const FitnessDepth& lowCutoff,
    const ConstEnvironmentPossible& origin) const {
  bool cutoff = base_t::testAgentSelection(bestOfWorst, worst, lowCutoff, origin);
  if (cutoff) { 
    orderHeuristic_.increment(origin, agentTeam_, worst.agentAction);
  }

  return cutoff;
}


bool PlannerMiniMax::testOtherSelection(
    EvalResult& worst,
    const EvalResult& current,
    const FitnessDepth& highCutoff,
    const ConstEnvironmentPossible& origin) const {
  bool cutoff = base_t::testOtherSelection(worst, current, highCutoff, origin);
  if (cutoff) { 
    orderHeuristic_.increment(origin, otherTeam_, current.otherAction);
  }

  return cutoff;
}


EvalResult PlannerMiniMax::recurse_alphabeta(
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
    result = base_t::recurse_alphabeta(origin, iDepth, lowCutoff, highCutoff, nodesEvaluated);
    transpositionTable_.put(origin.getHash(), result);
  }

  return result;
}
