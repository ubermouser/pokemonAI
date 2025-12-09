#ifndef RANDOM_EVALUATOR_H
#define RANDOM_EVALUATOR_H

#include "evaluator.h"

class EvaluatorRandom: public Evaluator {
public:

  ~EvaluatorRandom() { };
  EvaluatorRandom(const Config& cfg = Config{}): Evaluator(cfg) { };

  EvaluatorRandom* clone() const override { return new EvaluatorRandom(*this); }
protected:
  EvalResult calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;

  virtual std::string baseName() const override { return "Random"; }
};

#endif /* RANDOM_EVALUATOR_H */
