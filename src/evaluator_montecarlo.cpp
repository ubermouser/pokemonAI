#include "../inc/evaluator_montecarlo.h"

#include "../inc/evaluator_random.h"
#include "../inc/pkCU.h"
#include "../inc/planner_random.h"

EvaluatorMonteCarlo::EvaluatorMonteCarlo(const Config& cfg)
    : Evaluator("MonteCarlo_Evaluator"), cfg_(cfg) {
  Game::Config gamecfg;
  gamecfg.maxMatches = cfg_.maxRollouts;
  gamecfg.maxPlies = cfg_.maxPlies;
  gamecfg.numThreads = cfg_.numThreads;
  gamecfg.allowUndefinedAgents = false;
  gamecfg.allowStateSelection = false;
  gamecfg.storeSubcomponents = false;
  gamecfg.verbosity = 0;
  game_ = std::make_shared<Game>(gamecfg);
  game_->setEvaluator(EvaluatorRandom())
      .setPlanner(0, PlannerRandom().setEngine(PkCU()))
      .setPlanner(1, PlannerRandom().setEngine(PkCU()));
}


EvaluatorMonteCarlo& EvaluatorMonteCarlo::setEnvironment(
      const std::shared_ptr<const EnvironmentNonvolatile>& env) {
  Evaluator::setEnvironment(env);

  if (game_ != NULL) { game_->setEnvironment(env); }
  return *this;
}


EvaluatorMonteCarlo& EvaluatorMonteCarlo::initialize() {
  Evaluator::initialize();

  game_->initialize();
  return *this;
}


EvalResult_t EvaluatorMonteCarlo::calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const {
  // perform a rollout on each state:
  HeatResult result = game_->rollout(env);
  fpType agentFitness = result.score[iTeam];
  fpType otherFitness = result.score[(iTeam + 1) % 2];

  return EvalResult_t{combineTeamFitness(agentFitness, otherFitness), Action{}, Action{}};
}
