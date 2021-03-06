//#define PKAI_IMPORT
#include "../inc/evaluator_simple.h"

#include <algorithm>
#include <stdexcept>
#include <boost/format.hpp>

#include "../inc/engine.h"

namespace po = boost::program_options;


po::options_description EvaluatorSimple::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc = Evaluator::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "any-move-bias").c_str(),
      po::value<fpType>(&canMoveBias)->default_value(defaults.canMoveBias),
      "Individual move PP remaining bias")
      ((prefix + "move-bias").c_str(),
      po::value<fpType>(&movesBias)->default_value(defaults.movesBias),
      "Individual pokemon Move remaining bias")
      ((prefix + "alive-bias").c_str(),
      po::value<fpType>(&aliveBias)->default_value(defaults.aliveBias),
      "individual pokemon HP remaining bias")
      ((prefix + "any-alive-bias").c_str(),
      po::value<fpType>(&teamAliveBias)->default_value(defaults.teamAliveBias),
      "Any pokemon HP remaining bias");

  return desc;
}


EvaluatorSimple::EvaluatorSimple(const Config& cfg)
    : Evaluator(cfg), cfg_(cfg) {
  resetName();
};


EvaluatorSimple& EvaluatorSimple::initialize() {
  Evaluator::initialize();
  if (cfg_.aliveBias > 1.0 || cfg_.aliveBias < 0.0) { throw std::invalid_argument("aliveBias"); }
  if (cfg_.movesBias > 1.0 || cfg_.movesBias < 0.0) { throw std::invalid_argument("movesBias"); }
  if (cfg_.canMoveBias > 1.0 || cfg_.canMoveBias < 0.0) { throw std::invalid_argument("canMoveBias"); }
  if (cfg_.teamBias > 1.0 || cfg_.teamBias < 0.0) { throw std::invalid_argument("teamBias"); }
  if (cfg_.teamAliveBias > 1.0 || cfg_.teamAliveBias < 0.0) { throw std::invalid_argument("teamAliveBias"); }
  return *this;
}


void EvaluatorSimple::resetName() {
  setName((boost::format("%s(m=%3.1f)") % baseName() % cfg_.movesBias).str());
}


fpType EvaluatorSimple::fitness_move(const ConstMoveVolatile& mV) const {
  fpType fitness = (mV.getPercentPP() * (1. - cfg_.canMoveBias)) + (mV.hasPP() * (cfg_.canMoveBias));
  return fitness;
}


fpType EvaluatorSimple::fitness_pokemon(const ConstPokemonVolatile& pV) const {
  fpType hpBias = 1. - cfg_.movesBias - cfg_.aliveBias;
  bool isAlive = pV.isAlive();

  // accumulate move fitness:
  fpType moveAccumulator = 0.;
  for (size_t iMove = 0, count=isAlive?pV.nv().getNumMoves():0; iMove < count; ++iMove) {
    moveAccumulator += fitness_move(pV.getMV(iMove));
  }
  // TODO(@drendleman) does a valid pokemon have at least one move?
  moveAccumulator /= std::max(pV.nv().getNumMoves(), size_t(1U));

  fpType fitness =
      (moveAccumulator * cfg_.movesBias) +
      (isAlive * cfg_.aliveBias) +
      (pV.getPercentHP() * hpBias);
  return fitness;
}


fpType EvaluatorSimple::fitness_team(const ConstTeamVolatile& tV) const {
  bool isAlive = tV.isAlive();
  fpType pokemonAccumulator = 0.0;
  for (size_t iTeammate = 0, count = isAlive?tV.nv().getNumTeammates():0; iTeammate < count; ++iTeammate) {
    pokemonAccumulator += fitness_pokemon(tV.teammate(iTeammate));
  }
  pokemonAccumulator /= tV.nv().getNumTeammates();

  fpType fitness =
      (pokemonAccumulator * (1. - cfg_.teamAliveBias)) +
      (isAlive * cfg_.teamAliveBias);
  
  return fitness;
};


EvalResult EvaluatorSimple::calculateFitness(
    const ConstEnvironmentVolatile& env, size_t iTeam) const {
  // calculate fitness
  fpType agentFitness = fitness_team(env.getTeam(iTeam));
  fpType otherFitness = fitness_team(env.getOtherTeam(iTeam));

  return EvalResult{combineTeamFitness(agentFitness, otherFitness)};
};
