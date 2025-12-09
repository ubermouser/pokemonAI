//#define PKAI_IMPORT
#include "pokemonai/evaluator.h"

#include <algorithm>
#include <stdexcept>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "pokemonai/fitness.h"
#include "pokemonai/fp_compare.h"

namespace po = boost::program_options;


FitnessDepth::FitnessDepth(
    const Fitness& fitness_,
    const size_t depth_)
  : fitness(fitness_),
    depth(std::min(depth_, size_t(MAXTRIES))) {
}


po::options_description Evaluator::Config::options(const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "team-bias").c_str(),
      po::value<fpType>(&teamBias)->default_value(defaults.teamBias),
      "home team vs away team bias.");

  return desc;
}


void Evaluator::resetName() {
  setName(baseName());
}


Evaluator& Evaluator::initialize() {
  if (nv_ == NULL) { throw std::invalid_argument("evaluator nonvolatile environment undefined"); }
  if (cu_ == NULL) { throw std::invalid_argument("evaluator engine undefined"); }

  return *this;
}


Evaluator& Evaluator::setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv) {
  nv_ = nv;
  if (cu_ != NULL) { cu_->setEnvironment(nv); }
  return *this;
}


Evaluator& Evaluator::setEngine(const std::shared_ptr<PkCU>& cu) {
  cu_ = cu;
  if (nv_ != NULL) { cu_->setEnvironment(nv_); }
  return *this;
}


EvalResult Evaluator::evaluate(const ConstEnvironmentVolatile& env, size_t iTeam) const {
  auto gameState = cu_->getGameState(env);
  // perform a full evaluation if this is a midgame state:
  if (gameState == MATCH_MIDGAME) { 
    return calculateFitness(env, iTeam);
  }
  // this is a terminal state, evaluate terminal node:
  Fitness fitness =
      gameState==MATCH_TIE ? Fitness{0.5} :
      gameState==iTeam ? Fitness{1.0} : Fitness{0.0};

  // evaluation occurs at terminal depth:
  return EvalResult{fitness, Action{}, Action{}, MAXTRIES};
}


Fitness Evaluator::combineTeamFitness(fpType _agentFitness, fpType _otherFitness) const {
  // calculate fitness
  fpType agentFitness = (cfg_.teamBias)       * _agentFitness;
  fpType otherFitness = (1.0 - cfg_.teamBias) * _otherFitness;

  fpType maxFitness = std::max(agentFitness, otherFitness);
  // if maxFitness is about 0, we've tied the game. Ties do not favor either team
  if (mostlyEQ(maxFitness, 0.0)) { return Fitness{0.5}; }

  fpType fitness = (agentFitness - otherFitness) / maxFitness;

  // normalize fitness from 0..1 instead of -1..1
  fitness = (fitness + 1.0) / 2.0;
  return Fitness{fitness};
}
