//#define PKAI_IMPORT
#include "../inc/evaluator_simple.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "../inc/engine.h"


EvaluatorSimple::EvaluatorSimple(const Config& cfg)
    : Evaluator(cfg), cfg_(cfg) {
  std::ostringstream name;
  name << "Simple_Evaluator-" << cfg_.aliveBias;
  setName(name.str());
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


fpType EvaluatorSimple::fitness_move(const ConstMoveVolatile& mV) const {
  fpType fitness = (mV.getPercentPP() * (1. - cfg_.canMoveBias)) + (mV.hasPP() * (cfg_.canMoveBias));
  return fitness;
}


fpType EvaluatorSimple::fitness_pokemon(const ConstPokemonVolatile& pV) const {
  fpType hpBias = 1. - cfg_.movesBias - cfg_.aliveBias;
  bool isAlive = pV.isAlive();

  // accumulate move fitness:
  fpType moveAccumulator = 0.;
  for (size_t iMove = 0, count=isAlive?pV.nv().getNumMoves():0; isAlive && iMove < count; ++iMove) {
    moveAccumulator += fitness_move(pV.getMV(iMove));
  }
  // TODO(@drendleman) does a valid pokemon have at least one move?
  moveAccumulator /= std::max(pV.nv().getNumMoves(), 1U);

  fpType fitness =
      (moveAccumulator * cfg_.movesBias) +
      (isAlive * cfg_.aliveBias) +
      (pV.getPercentHP() * hpBias);
  return fitness;
}


fpType EvaluatorSimple::fitness_team(const ConstTeamVolatile& tV) const {
  bool isAlive = tV.isAlive();
  fpType pokemonAccumulator = 0.0;
  for (size_t iTeammate = 0, count = isAlive?tV.nv().getNumTeammates():0; iTeammate != count; ++iTeammate) {
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
