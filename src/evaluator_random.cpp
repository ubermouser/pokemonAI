//#define PKAI_IMPORT
#include "../inc/evaluator_random.h"

const std::string EvaluatorRandom::ident = "Random_Evaluator";

EvalResult_t EvaluatorRandom::calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) {
  EvalResult_t result = { 
    (fpType)rand() / (fpType) RAND_MAX,
    (int8_t)((rand() % AT_ITEM_USE+2) - 1),
    (int8_t)((rand() % AT_ITEM_USE+2) - 1)
  };
  return result;
};
