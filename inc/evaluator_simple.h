#ifndef SIMPLE_EVALUATOR_H
#define SIMPLE_EVALUATOR_H

#include "evaluator.h"

class EvaluatorSimple: public Evaluator
{
protected:
  fpType bias;
  
public:
  EvaluatorSimple(fpType _bias = 0.5);
  ~EvaluatorSimple() = default;
  EvaluatorSimple(const EvaluatorSimple& other) = default;

  static fpType fitness_team(const ConstTeamVolatile& tV);
  static fpType calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam, fpType bias = 0.5);

  EvaluatorSimple* clone() const override { return new EvaluatorSimple(*this); }

  bool isInitialized() const override;

  EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) override;
};

#endif /* SIMPLE_EVALUATOR_H */
