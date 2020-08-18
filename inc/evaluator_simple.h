#ifndef SIMPLE_EVALUATOR_H
#define SIMPLE_EVALUATOR_H

#include "evaluator.h"

class evaluator_simple: public Evaluator
{
protected:
  fpType bias;
  
public:
  evaluator_simple() = delete;
  evaluator_simple(fpType _bias = 0.5);
  ~evaluator_simple() = default;
  evaluator_simple(const evaluator_simple& other) = default;

  static fpType fitness_team(const ConstTeamVolatile& tV);
  static fpType calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam, fpType bias = 0.5);

  evaluator_simple* clone() const override { return new evaluator_simple(*this); }

  bool isInitialized() const override;

  EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) override;
};

#endif /* SIMPLE_EVALUATOR_H */
