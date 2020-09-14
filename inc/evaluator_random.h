#ifndef RANDOM_EVALUATOR_H
#define RANDOM_EVALUATOR_H

#include "evaluator.h"

class EvaluatorRandom: public Evaluator {
public:

  ~EvaluatorRandom() { };
  EvaluatorRandom(const Config& cfg = Config{}): Evaluator("Random_Evaluator", cfg) { };

  EvaluatorRandom* clone() const override { return new EvaluatorRandom(*this); }

  EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;
protected:
  static const std::string ident;
};

#endif /* RANDOM_EVALUATOR_H */
