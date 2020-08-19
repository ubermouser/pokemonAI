#ifndef RANDOM_EVALUATOR_H
#define RANDOM_EVALUATOR_H

#include "evaluator.h"

class EvaluatorRandom: public Evaluator
{
private:
  static const std::string ident;
public:

  ~EvaluatorRandom() { };
  EvaluatorRandom(): Evaluator("Random_Evaluator") { };

  EvaluatorRandom* clone() const { return new EvaluatorRandom(*this); }

  EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam);
};

#endif /* RANDOM_EVALUATOR_H */
