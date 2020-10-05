//#define PKAI_IMPORT
#include "../inc/planner.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

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
  if (agentTeam_ >= 2) { throw std::invalid_argument("planner agentTeam undefined"); }
  otherTeam_ = (agentTeam_ + 1) % 2;
  
  if (nv_ == NULL) { throw std::invalid_argument("planner nonvolatile environment undefined"); }
  if (cu_ == NULL) { throw std::invalid_argument("planner engine undefined"); }
  if (eval_ == NULL && isEvaluatorRequired()) { throw std::invalid_argument("planner evaluator undefined"); }
  if (eval_ != NULL) { eval_->initialize(); }

  if (cfg_.maxDepth > maxImplDepth()) {
    std::cerr <<
        getName() << " evaluator has maximum implementation depth of " <<
        maxImplDepth() << ", ignoring depth " <<
        cfg_.maxDepth << "!\n";
    cfg_.maxDepth = maxImplDepth();
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
  std::string evalName = ((eval_!= NULL)?(boost::format("-%s") % eval_->getName()).str(): "");
  std::string planName = (boost::format("%s(d=%d)%s")
      % baseName()
      % cfg_.maxDepth
      % evalName).str();
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
  for (size_t iDepth = 0; iDepth <= cfg_.maxDepth; ++iDepth) {
    PlyResult plyResult;
    if (iDepth == 0) {
      plyResult = generateSolutionAtLeaf(origin);
    } else {
      plyResult = generateSolutionAtDepth(origin, iDepth);
    }
    iDepth = std::max(iDepth, size_t(plyResult.depth));

    // determine time between last checkpoint and current checkpoint:
    auto checkpoint = std::chrono::steady_clock::now();
    plyResult.timeSpent =  std::chrono::duration<double>(checkpoint - start).count();
    result.atDepth.push_back(plyResult);

    bool hasSolution = result.hasSolution();
    bool terminalDepth = (iDepth >= cfg_.maxDepth);
    bool terminalTime = plyResult.timeSpent > cfg_.maxTime;
    printSolution(result, terminalDepth || terminalTime);

    // break loop early if we are over maximum time
    if (hasSolution && terminalTime) { break; }
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
    const Action& agentAction,
    const Action& otherAction) const {
  // produce the resulting state of iAction:
  const Action& team_a_action = agentTeam_==TEAM_A?agentAction:otherAction;
  const Action& team_b_action = agentTeam_==TEAM_B?agentAction:otherAction;
  PossibleEnvironments rEnvP = cu_->updateState(origin, team_a_action, team_b_action);

  return rEnvP;
}


ActionVector Planner::getValidActions(
    const ConstEnvironmentPossible& origin,
    size_t iTeam) const {
  return cu_->getValidActions(origin, iTeam);
}


bool Planner::testAgentCutoff(
    EvalResult& bestOfWorst,
    const EvalResult& worst,
    const ConstEnvironmentPossible& origin) const {
  // TODO(@drendleman) in the case of a tie, the agent should always bias towards a damaging action
  if (worst > bestOfWorst) {
    bestOfWorst = worst;
    return true;
  }
  return false;
}


bool Planner::testOtherCutoff(
    EvalResult& worst,
    const EvalResult& current,
    const ConstEnvironmentPossible& origin) const {
  if (current < worst) {
    worst = current;
    return true;
  }
  return false;
}


EvalResult Planner::recurse_alphabeta(
      const ConstEnvironmentPossible& origin,
      size_t iDepth,
      const Fitness& lowCutoff,
      const Fitness& highCutoff,
      size_t* nodesEvaluated) const {
  // the best agent move fitness:
  EvalResult bestOfWorst{lowCutoff};

  // TODO(@drendleman) - evaluate these in accordance to butterfly heuristic:
  // for every possible move by both agent team and other team:
  for (const auto& agentAction: getValidActions(origin, agentTeam_)) {
    EvalResult worst{highCutoff};
    // the worst possible other team choice is the one which causes the agent to decide to use it:
    for (const auto& otherAction: getValidActions(origin, otherTeam_)) {
      // evaluate what probabilistically will occur if agent and other teams perform action at state:
      FitnessDepth child = recurse_gamma(
          origin,
          agentAction,
          otherAction,
          iDepth - 1,
          bestOfWorst.fitness,
          worst.fitness,
          nodesEvaluated);
      // TODO(@drendleman) mark that the node is fully evaluated
      // has the other agent improved upon its best score by reducing our score more?
      testOtherCutoff(
          worst, EvalResult{child.fitness, agentAction, otherAction, child.depth + 1}, origin);
    }
    
    // is the min of all other agent moves better than the best of our current moves?
    testAgentCutoff(bestOfWorst, worst, origin);
  }

  return bestOfWorst;
}


FitnessDepth Planner::recurse_gamma(
      const ConstEnvironmentPossible& origin,
      const Action& agentAction,
      const Action& otherAction,
      size_t iDepth,
      const Fitness& lowCutoff,
      const Fitness& highCutoff,
      size_t* nodesEvaluated) const {
  size_t numNodes = 0;
  // average fitness of all states combined:
  FitnessDepth result{Fitness{0., 0.}, MAXTRIES};
  auto rEnvP = generateStates(origin, agentAction, otherAction);

  // TODO(@drendleman) - evaluate these in order of greatest probability to least
  for (const auto& cEnvP : rEnvP.getValidEnvironments(true)) {
    // the likelihood that this state occurs:
    fpType stateProbability = cEnvP.getProbability().to_double();
    // this individual state's fitness:
    EvalResult child;

    // if this is either a terminal node or if we are at terminal depth:
    bool isGameOver = cu_->isGameOver(cEnvP);
    if (isGameOver || iDepth == 0) {
      // evaluate as a leaf node:
      child = eval_->evaluate(cEnvP, agentTeam_);
      ++numNodes;
    } else { // else, recurse to a deeper level:
      // recurse into another depth, widening the cutoffs by the probability of the move:
      child = recurse_alphabeta(
          cEnvP,
          iDepth,
          lowCutoff.expand(stateProbability),
          highCutoff.expand(stateProbability),
          nodesEvaluated);
    }

    // reduce the certainty of the deeper fitness result by state's occurrence probability, and
    //  accumulate:
    result.fitness += child.fitness.expand(stateProbability);
    result.depth = std::min(result.depth, child.depth);

    // TODO(@drendleman) cutoff when solution has been found at a shallower depth
    // if there's no possibility this action is the best for the agent, do not continue:
    if (result.fitness < lowCutoff) { break; }
    // if the other team would never choose this move against the agent, do not continue:
    if (result.fitness > highCutoff) { break; }
  }

  if (nodesEvaluated != NULL) { *nodesEvaluated += numNodes; }
  return result;
}


void Planner::printSolution(const PlannerResult& results, bool isLast) const {
  if (cfg_.verbosity < (isLast?1:2)) { return; }

  std::stringstream out;
  if (!results.atDepth.empty()) {
    const auto& result = results.best();

    out << boost::format("%sT%s: ply=%2d act=%4s oact=%4s fit=% 6.4f time=%6.2f nnod=%d\n")
        % (isLast?"~~~~":"    ")
        % (agentTeam_==TEAM_A?"A":"B")
        % result.depth
        % result.agentAction
        % result.otherAction
        % result.fitness.lowerBound()
        % result.timeSpent
        % result.numNodes;
  } else {
    out << "~~~~T" << (agentTeam_==TEAM_A?"A":"B") <<
      ": NO SOLUTIONS FOUND FOR ANY DEPTH!\n";
  }
  std::cout << out.str();
}
