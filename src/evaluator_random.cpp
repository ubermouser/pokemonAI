//#define PKAI_IMPORT
#include "../inc/evaluator_random.h"

const std::string EvaluatorRandom::ident = "Random_Evaluator";

EvalResult_t EvaluatorRandom::calculateFitness(
    const ConstEnvironmentVolatile& env, size_t iTeam) const {
  // TODO(@drendleman) choose valid actions?
  EvalResult_t result = { 
    (fpType)rand() / (fpType) RAND_MAX,
    (Action)((rand() % AT_ITEM_USE+2) - 1),
    (Action)((rand() % AT_ITEM_USE+2) - 1)
  };
  return result;
};
