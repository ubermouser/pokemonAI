//#define PKAI_IMPORT
#include "pokemonai/evaluator_random.h"


EvalResult EvaluatorRandom::calculateFitness(
    const ConstEnvironmentVolatile& env, size_t iTeam) const {
  // TODO(@drendleman) choose valid actions?
  // TODO(@drendleman) terminal fitness?
  EvalResult result = {
      Fitness{std::clamp(
          (fpType)rand() / (fpType)RAND_MAX, (fpType)0.0, (fpType)1.0)},
      Action{rand() % (Action::MOVE_LAST)},
      Action{rand() % (Action::MOVE_LAST)}};
  return result;
};
