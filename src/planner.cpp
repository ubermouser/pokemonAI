//#define PKAI_IMPORT
#include "pokemonai/planner.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <boost/program_options.hpp>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace po = boost::program_options;


po::options_description Planner::Config::options(const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "planner-verbosity").c_str(),
      po::value<int>(&verbosity)->default_value(defaults.verbosity),
      "verbosity level, controls intermediate ply result printing.")
      ((prefix + "max-search-time").c_str(),
      po::value<double>(&maxTime)->default_value(defaults.maxTime),
      "Maximum planner evaluation search time.")
      ((prefix + "max-search-depth").c_str(),
      po::value<size_t>(&maxDepth)->default_value(defaults.maxDepth),
      "Maximum planner evaluation search depth.");

  return desc;

}


Planner& Planner::initialize() {
  otherTeam_ = (TEAM)((agentTeam_ + 1) % 2);

  if (nv_ == NULL) { throw std::invalid_argument("planner nonvolatile environment undefined"); }
  if (cu_ == NULL) { throw std::invalid_argument("planner engine undefined"); }
  if (eval_ == NULL && isEvaluatorRequired()) { throw std::invalid_argument("planner evaluator undefined"); }
  if (eval_ != NULL) { eval_->initialize(); }

  if (cfg_.maxDepth > maxImplDepth()) {
    SPDLOG_WARN("{} planner has maximum implementation depth of {}, ignoring depth {}!", 
        getName(), maxImplDepth(), cfg_.maxDepth);
    cfg_.maxDepth = maxImplDepth();
  }
  if (cfg_.minDepth > cfg_.maxDepth) {
    cfg_.minDepth = cfg_.maxDepth;
  }

  return *this;
}


Planner& Planner::setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv) {
  nv_ = nv;
  if (cu_ != NULL) { cu_->setEnvironment(nv); }
  if (eval_ != NULL) { eval_->setEnvironment(nv); }
  return *this;
}


Planner& Planner::setEngine(const std::shared_ptr<PkCU>& cu) {
  cu_ = cu;
  if (nv_ != NULL) { cu_->setEnvironment(nv_); }
  if (eval_ != NULL) { eval_->setEngine(cu_); }
  return *this;
}


Planner& Planner::setEvaluator(const std::shared_ptr<Evaluator>& eval) {
  eval_ = eval;
  if (nv_ != NULL) { eval_->setEnvironment(nv_); }
  if (cu_ != NULL) { eval_->setEngine(cu_); }
  resetName();
  return *this;
}


void Planner::resetName() {
  std::string evalName =
      ((eval_ != NULL) ? fmt::format("-{}", eval_->getName()) : "");
  std::string planName =
      fmt::format("{}(d={}){}", baseName(), cfg_.maxDepth, evalName);
  setName(planName);
}


PlannerResult Planner::generateSolution(const ConstEnvironmentVolatile& origin) const {
  auto data = EnvironmentPossibleData::create(origin.data());
  return generateSolution(ConstEnvironmentPossible{origin.nv(), data});
}


PlannerResult Planner::generateSolution(const ConstEnvironmentPossible& origin) const {
  PlannerResult result; result.atDepth.reserve(cfg_.maxDepth);
  // keep an elapsed time counter
  auto start = std::chrono::steady_clock::now();

  // evaluate 0..nth state:
  for (size_t iDepth = cfg_.minDepth; iDepth <= cfg_.maxDepth; ++iDepth) {
    PlyResult plyResult;
    if (iDepth == 0) {
      plyResult = generateSolutionAtLeaf(origin);
    } else {
      plyResult = generateSolutionAtDepth(origin, iDepth);
    }

    // determine time between last checkpoint and current checkpoint:
    auto checkpoint = std::chrono::steady_clock::now();
    plyResult.timeSpent =  std::chrono::duration<double>(checkpoint - start).count();
    result.atDepth.push_back(plyResult);

    bool hasSolution = result.hasSolution();
    // increasing the search depth did not lead to the search deepening:
    bool terminalDepth  = plyResult.depth < iDepth;
    // a memoized evaluation has a depth greater than what we asked for:
    bool memoizedDepth = plyResult.depth >= cfg_.maxDepth;
    // we're out of time:
    bool terminalTime = plyResult.timeSpent > cfg_.maxTime;
    printSolution(result, terminalDepth || memoizedDepth || terminalTime);

    // break loop early if we are over maximum time or cannot evaluate further:
    if (hasSolution && (terminalTime || terminalDepth)) { break; }

    iDepth = std::max(iDepth, size_t(plyResult.depth));
  }

  return result;
}


PlyResult Planner::generateSolutionAtDepth(
    const ConstEnvironmentPossible& origin, size_t maxDepth) const {
  throw std::logic_error("not implemented");
}


PlyResult Planner::generateSolutionAtLeaf(const ConstEnvironmentPossible& origin) const {
  assert(eval_ != NULL);

  PlyResult result = eval_->evaluate(origin, agentTeam_);
  result.numNodes = 1;

  return result;
}


PossibleEnvironments Planner::generateStates(
    const ConstEnvironmentPossible& origin,
    const ActionMap& agentAction,
    const ActionMap& otherAction) const {
  // produce the resulting state of iAction:
  const ActionMap& team_a_action =
      agentTeam_ == TEAM_A ? agentAction : otherAction;
  const ActionMap& team_b_action =
      agentTeam_ == TEAM_B ? agentAction : otherAction;
  PossibleEnvironments rEnvP =
      cu_->updateState(origin.getEnv(), team_a_action, team_b_action);

  return rEnvP;
}


std::vector<ActionMap> Planner::getValidActions(
    const ConstEnvironmentPossible& origin, TEAM iTeam) const {
  return cu_->getAllValidActions(origin.getEnv(), iTeam);
}


EvalResult Planner::recurse_alphabeta(
      const ConstEnvironmentPossible& origin,
      size_t searchDepth,
      const FitnessDepth& _lowCutoff,
      const FitnessDepth& _highCutoff,
      size_t* nodesEvaluated) const {
  // the best agent move fitness:
  EvalResult bestOfWorst{Fitness::worst()};

  // for every possible move by the agent team:
  for (const auto& agentAction: getValidActions(origin, agentTeam_)) {
    const FitnessDepth& lowCutoff = std::max((const FitnessDepth&)bestOfWorst, _lowCutoff); // low cutoff (cannot do worse)

    // find the worst possible outcome for this agent action by looking at all
    // other team responses:
    EvalResult worst = recurse_beta(
        origin,
        agentAction,
        searchDepth,
        lowCutoff,
        _highCutoff,
        nodesEvaluated);

    // is the min of all other agent moves better than the best of our current
    // moves?
    if (testAgentSelection(bestOfWorst, worst, _highCutoff, origin)) {
      SPDLOG_TRACE(
          "Agent selection cutoff! c={} high={}",
          fmt::streamed(bestOfWorst.fitness),
          fmt::streamed(_highCutoff.fitness));
      break;
    };
  }  // endOf foreach agent move

  return bestOfWorst;
}


EvalResult Planner::recurse_beta(
    const ConstEnvironmentPossible& origin,
    const ActionMap& agentAction,
    size_t searchDepth,
    const FitnessDepth& lowCutoff,
    const FitnessDepth& _highCutoff,
    size_t* nodesEvaluated) const {
  EvalResult worst{Fitness::best()};

  // the worst possible other team choice is the one which causes the agent to
  // decide to use it:
  for (const auto& otherAction : getValidActions(origin, otherTeam_)) {
    const FitnessDepth& highCutoff = std::min(
        (const FitnessDepth&)worst,
        _highCutoff);  // high cutoff (cannot do better)

    // evaluate what probabilistically will occur if agent and other teams
    // perform action at state:
    EvalResult child = recurse_gamma(
        origin,
        agentAction,
        otherAction,
        searchDepth - 1,
        lowCutoff,   // low cutoff (cannot do worse)
        highCutoff,  // high cutoff (cannot do better)
        nodesEvaluated);

    // has the other agent improved upon its best score by reducing our score
    // more?
    if (testOtherSelection(worst, child, lowCutoff, origin)) {
      SPDLOG_TRACE(
          "Other selection cutoff! c={} low={}",
          fmt::streamed(worst.fitness),
          fmt::streamed(lowCutoff.fitness));
      break;
    }
  }  // endOf foreach other move

  return worst;
}


EvalResult Planner::recurse_gamma(
    const ConstEnvironmentPossible& origin,
    const ActionMap& agentAction,
    const ActionMap& otherAction,
    size_t searchDepth,
    const FitnessDepth& lowCutoff,
    const FitnessDepth& highCutoff,
    size_t* nodesEvaluated) const {
  assert(searchDepth <= MAXTRIES && searchDepth >= 0);
  size_t numNodes = 0;
  // average fitness of all states combined:
  EvalResult result{
      Fitness{fpType(0), Fitness::prob_zero()}, agentAction, otherAction};

  auto rEnvP = generateStates(origin, agentAction, otherAction);
  for (const auto& cEnvP : rEnvP.getValidEnvironments(true)) {
    // the likelihood that this state occurs:
    FixType stateProbability = cEnvP.getProbability();
    // this individual state's fitness:
    EvalResult child;

    // if this is either a terminal node or if we are at terminal depth:
    bool isGameOver = cu_->isGameOver(cEnvP);
    if (isGameOver || searchDepth == 0) {
      // evaluate as a leaf node:
      child = eval_->evaluate(cEnvP, agentTeam_);
      ++numNodes;
    } else { // else, recurse to a deeper level:
      // recurse into another depth, widening the cutoffs by the probability of the move:
      child = recurse_alphabeta(
          cEnvP,
          searchDepth,
          lowCutoff.expand(stateProbability),
          highCutoff.expand(stateProbability),
          nodesEvaluated);
    }

    // reduce the certainty of the deeper fitness result by state's occurrence probability, and
    //  accumulate:
    result.fitness += child.fitness.expand(stateProbability);
    // TODO(@drendleman) - this can sometimes produce weird depths, why?
    result.depth = std::max(result.depth, child.depth);

    // if this action will never be chosen by either agent or other, do not continue:
    if (testGammaCutoff(result, lowCutoff, highCutoff)) {
      SPDLOG_TRACE(
          "Gamma cutoff! c={} low={} high={} ",
          fmt::streamed(result.fitness),
          fmt::streamed(lowCutoff.fitness),
          fmt::streamed(highCutoff.fitness));
      break;
    }
  } // endOf foreach environment

  if (cfg_.verbosity >= 4) {
    printStateEvaluation(origin, searchDepth, result, numNodes);
  }
  if (nodesEvaluated != NULL) { *nodesEvaluated += numNodes; }
  return EvalResult{result.fitness, agentAction, otherAction, result.depth + 1};
}


void Planner::printStateEvaluation(
    const ConstEnvironmentPossible& origin,
    size_t searchDepth,
    const EvalResult& evalResult,
    size_t numNodes) const {
  std::string padding(searchDepth, ' ');
  fmt::print(
      "{}T{}: s=x{:06x} i={:2} a={:4} o={:4} d={:2} n={} {}\n",
      padding,
      (agentTeam_ == TEAM_A ? "A" : "B"),
      (origin.getHash() & 0xffffff),
      searchDepth,
      fmt::streamed(evalResult.agentAction),
      fmt::streamed(evalResult.otherAction),
      evalResult.depth,
      numNodes,
      fmt::streamed(evalResult.fitness));
}


void Planner::printSolution(const PlannerResult& results, bool isLast) const {
  if (cfg_.verbosity < (isLast ? 1 : 2)) { return; }

  if (!results.atDepth.empty()) {
    const auto& result = results.best();

    fmt::print(
        "{}T{}: ply={:2} act={:4} oact={:4} fit={:6.4f} time={:6.2f} "
        "nnod={}\n",
        (isLast ? "~~~~" : "    "),
        (agentTeam_ == TEAM_A ? "A" : "B"),
        result.depth,
        fmt::streamed(result.agentAction),
        fmt::streamed(result.otherAction),
        result.fitness.lowerBound(),
        result.timeSpent,
        result.numNodes);
  } else {
    fmt::print(
        "~~~~T{}: NO SOLUTIONS FOUND FOR ANY DEPTH!\n",
        (agentTeam_ == TEAM_A ? "A" : "B"));
  }
}
