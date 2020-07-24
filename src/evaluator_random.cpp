//#define PKAI_IMPORT
#include "../inc/evaluator_random.h"

const std::string evaluator_random::ident = "Random_Evaluator";

evalResult_t evaluator_random::calculateFitness(const environment_volatile& env, size_t iTeam)
{
  evalResult_t result = { (fpType)rand() / (fpType) RAND_MAX, (rand() % AT_ITEM_USE+2) - 1, (rand() % AT_ITEM_USE+2) - 1 };
  return result;
};
