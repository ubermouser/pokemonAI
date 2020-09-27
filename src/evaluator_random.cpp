//#define PKAI_IMPORT
#include "../inc/evaluator_random.h"


EvalResult EvaluatorRandom::calculateFitness(
    const ConstEnvironmentVolatile& env, size_t iTeam) const {
  // TODO(@drendleman) choose valid actions?
  // TODO(@drendleman) terminal fitness?
  EvalResult result = {
    Fitness{(fpType)rand() / (fpType) RAND_MAX},
    Action{rand() % (Action::MOVE_LAST)},
    Action{rand() % (Action::MOVE_LAST)}
  };
  return result;
};
