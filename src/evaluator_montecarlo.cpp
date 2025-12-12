#include "pokemonai/evaluator_montecarlo.h"

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "pokemonai/evaluator_simple.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/planner_random.h"

namespace po = boost::program_options;


po::options_description EvaluatorMonteCarlo::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc = Evaluator::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "move-chance").c_str(),
      po::value<double>(&moveChance)->default_value(defaults.moveChance),
      "likelihood for the random action selected to be a move.")
      ((prefix + "max-rollouts").c_str(),
      po::value<size_t>(&maxRollouts)->default_value(defaults.maxRollouts),
      "maximum number of rollouts per calculation.")
      ((prefix + "max-plies").c_str(),
      po::value<size_t>(&maxPlies)->default_value(defaults.maxPlies),
      "Maximum depth per rollout.")
      ((prefix + "num-threads").c_str(),
      po::value<size_t>(&numThreads)->default_value(defaults.numThreads),
      "If greater than 0, thread-parallelism is used.");

  return desc;
}


EvaluatorMonteCarlo* EvaluatorMonteCarlo::clone() const {
  EvaluatorMonteCarlo* newEval = new EvaluatorMonteCarlo(*this);
  if (game_) {
    newEval->game_ = std::shared_ptr<Game>(game_->clone());
  }
  return newEval;
}


EvaluatorMonteCarlo::EvaluatorMonteCarlo(const Config& cfg)
    : Evaluator(cfg), cfg_(cfg) {
  resetName();
}


void EvaluatorMonteCarlo::resetName() {
  setName((boost::format("%s(r=%d)") % baseName() % cfg_.maxRollouts).str());
}


EvaluatorMonteCarlo& EvaluatorMonteCarlo::setEnvironment(
      const std::shared_ptr<const EnvironmentNonvolatile>& env) {
  Evaluator::setEnvironment(env);

  if (game_) {
    game_->setEnvironment(env);
  }
  return *this;
}


EvaluatorMonteCarlo& EvaluatorMonteCarlo::setEngine(const std::shared_ptr<PkCU>& cu) {
  Evaluator::setEngine(cu);

  if (game_) {
    game_->setEngine(cu);
  }
  return *this;
}


EvaluatorMonteCarlo& EvaluatorMonteCarlo::initialize() {
  Evaluator::initialize();

  PlannerRandom::Config plannercfg;
  plannercfg.moveChance = cfg_.moveChance;
  Game::Config gamecfg;
  gamecfg.maxMatches = cfg_.maxRollouts;
  gamecfg.maxPlies = cfg_.maxPlies;
  gamecfg.numThreads = 0;
  gamecfg.allowUndefinedAgents = false;
  gamecfg.allowStateSelection = false;
  gamecfg.storeSubcomponents = false;
  gamecfg.verbosity = 0;
  game_ = std::make_shared<Game>(gamecfg);
  game_->setEvaluator(EvaluatorSimple().setEngine(*cu_))
      .setPlanner(0, PlannerRandom(plannercfg).setEngine(*cu_))
      .setPlanner(1, PlannerRandom(plannercfg).setEngine(*cu_));

  game_->initialize();
  return *this;
}


EvalResult EvaluatorMonteCarlo::calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const {
  // perform a rollout on each state:
  HeatResult result = game_->rollout(env);
  fpType fitness = result.teams[iTeam].lastSimpleFitness;

  return EvalResult{Fitness{fitness}};
}
