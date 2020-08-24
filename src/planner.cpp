//#define PKAI_IMPORT
#include "../inc/planner.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <chrono>

#include <boost/program_options.hpp>

namespace po = boost::program_options;


po::options_description Planner::Config::options(
    Config& cfg, const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "planner-verbosity").c_str(),
      po::value<int>(&cfg.verbosity)->default_value(defaults.verbosity),
      "verbosity level, controls intermediate ply result printing.")
      ((prefix + "max-search-time").c_str(),
      po::value<double>(&cfg.maxTime)->default_value(defaults.maxTime),
      "Maximum planner evaluation search time.")
      ((prefix + "max-search-depth").c_str(),
      po::value<size_t>(&cfg.maxDepth)->default_value(defaults.maxDepth),
      "Maximum planner evaluation search depth.");

  return desc;

}


bool Planner::isInitialized() const {
    if (agentTeam_ >= 2) { return false; }
    if (nv_ == NULL) { return false; }
    if (cu_ == NULL) { return false; }
    if (eval_ == NULL && isEvaluatorRequired()) { return false; }
    if (eval_ != NULL && !eval_->isInitialized()) { return false; }

    if (cfg_.maxDepth > maxImplDepth()) {
      std::cerr <<
          getName() << " evaluator has maximum implementation depth of " <<
          maxImplDepth() << ", ignoring depth " <<
          cfg_.maxDepth << "!\n";
      return false;
    }

    return true;
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
  return *this;
}


Planner& Planner::setEvaluator(const std::shared_ptr<Evaluator>& eval) {
  eval_ = eval;
  if (nv_ != NULL) { eval_->setEnvironment(nv_); }
  resetName();
  return *this;
}


void Planner::resetName() {
  std::ostringstream name;
  name << baseName();
  name << "-";
  name << ((eval_!= NULL) ? eval_->getName(): "NULLEVAL");
  setName(name.str());
}


PlannerResult Planner::generateSolution(const ConstEnvironmentVolatile& origin) const {
  PlannerResult result; result.atDepth.reserve(cfg_.maxDepth);
  // keep an elapsed time counter
  auto start = std::chrono::steady_clock::now();
  for (size_t iDepth = 1; iDepth <= cfg_.maxDepth; ++iDepth) {
    auto plyResult = generateSolutionAtDepth(origin, iDepth);
    plyResult.depth = iDepth;

    // determine time between last checkpoint and current checkpoint:
    auto checkpoint = std::chrono::steady_clock::now();
    plyResult.timeSpent =  std::chrono::duration<double>(checkpoint - start).count();
    result.atDepth.push_back(plyResult);

    printSolution(result, iDepth == cfg_.maxDepth);

    // break loop early if we are over maximum time
    if (plyResult.timeSpent > cfg_.maxTime) { break; }
  }

  return result;
}


PossibleEnvironments Planner::generateStates(
    const ConstEnvironmentVolatile& origin,
    const Action& agentAction,
    const Action& otherAction) const {
  // produce the resulting state of iAction:
  const Action& team_a_action = agentTeam_==TEAM_A?agentAction:otherAction;
  const Action& team_b_action = agentTeam_==TEAM_B?agentAction:otherAction;
  PossibleEnvironments rEnvP = cu_->updateState(origin, team_a_action, team_b_action);

  return rEnvP;
}


Fitness Planner::recurse_alphabeta(
      const ConstEnvironmentVolatile& origin,
      size_t iDepth,
      const Fitness& lowCutoff,
      const Fitness& highCutoff,
      size_t* nodesEvaluated) const {
  Fitness low = lowCutoff;
  std::unordered_map<Action, Fitness> fitnesses;

  // TODO(@drendleman) - evaluate these in accordance to butterfly heuristic:
  for (const auto& actions: cu_->getAllValidActions(origin, agentTeam_)) {
    if (fitnesses.count(actions[0]) == 0) { fitnesses[actions[0]] = highCutoff; }
    auto& high = fitnesses[actions[0]];

    // go deeper to evaluate a gamma node:
    Fitness fitness;
    fitness = recurse_gamma(
          origin, actions[0], actions[1], iDepth - 1, low, high, nodesEvaluated);

    // TODO(@drendleman) in the case of a tie, the agent should always bias towards a damaging action
    // has the other agent improved upon its best score by reducing our score more?
    if (fitness < high) {
      high = fitness;
    }
    // is the min of all other agent moves better than the best of our current moves?
    if (high > low) {
      low = high;
    }
  }

  return low;
}


Fitness Planner::recurse_gamma(
      const ConstEnvironmentVolatile& origin,
      const Action& agentAction,
      const Action& otherAction,
      size_t iDepth,
      const Fitness& lowCutoff,
      const Fitness& highCutoff,
      size_t* nodesEvaluated) const {
  size_t numNodes = 0;
  Fitness fitness;
  auto rEnvP = generateStates(origin, agentAction, otherAction);

  // TODO(@drendleman) - evaluate these in order of greatest probability to least
  for (const auto& cEnvP : rEnvP.getValidEnvironments()) {
    fpType stateProbability = cEnvP.getProbability().to_double();

    // if this is either a terminal node or if we are at terminal depth:
    if (cu_->isGameOver(cEnvP) || iDepth == 0) {
      // evaluate as a leaf node:
      EvalResult_t evalResult = eval_->calculateFitness(cEnvP, agentTeam_);
      fitness += Fitness{evalResult.fitness, stateProbability};
      ++numNodes;
    } else { // else, recurse to a deeper level:
      // recurse into another depth, widening the cutoffs by the probability of the move:
      Fitness deeperFitness = recurse_alphabeta(
          cEnvP,
          iDepth,
          lowCutoff.expand(stateProbability),
          highCutoff.expand(stateProbability),
          nodesEvaluated);
      // reduce the certainty of the deeper fitness result by the probability of the move:
      fitness += deeperFitness.expand(stateProbability);
    }

    // if there's no possibility this action is the best for the agent, do not continue:
    if (fitness < lowCutoff) { break; }
    // if the other team would never choose this move against the agent, do not continue:
    if (fitness > highCutoff) { break; }
  }

  if (nodesEvaluated != NULL) { *nodesEvaluated += numNodes; }
  return fitness;
}


void Planner::printSolution(const PlannerResult& results, bool isLast) const {
  if (cfg_.verbosity < (isLast?1:2)) { return; }
  
  if (!results.atDepth.empty()) {
    const auto& result = results.best();

    std::clog << (isLast?"~~~~T":"    T") << (agentTeam_==TEAM_A?"A":"B") <<
      ": ply=" << std::setw(2) << result.depth <<
      " act=" << std::setw(2) << result.agentAction <<
      " oact=" << std::setw(2) << result.otherAction <<
      " fit=" << std::setw(9) << result.fitness.lowerBound() <<
      " elaps=" << std::setw(7) << result.timeSpent << "s" <<
      " nnod=" << std::dec << result.numNodes <<
      "\n";

  } else {
    std::clog << "~~~~T" << (agentTeam_==TEAM_A?"A":"B") <<
      ": NO SOLUTIONS FOUND FOR ANY DEPTH!\n";
  }
}
