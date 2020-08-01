#ifndef SIMPLE_EVALUATOR_H
#define SIMPLE_EVALUATOR_H

#include "../inc/evaluator.h"

class evaluator_simple: public Evaluator
{
private:
  std::string ident;

  fpType bias;

  const EnvironmentNonvolatile* envNV;
public:
  ~evaluator_simple() { };
  evaluator_simple(fpType _bias = 0.5);
  evaluator_simple(const evaluator_simple& other);

  static fpType fitness_team(const TeamVolatile& tV, const TeamNonVolatile& tNV);
  static fpType calculateFitness(const EnvironmentNonvolatile& envNV, const EnvironmentVolatile& env, size_t iTeam, fpType bias = 0.5);

  const std::string& getName() const { return ident; };

  evaluator_simple* clone() const { return new evaluator_simple(*this); }

  bool isInitialized() const;

  void resetEvaluator(const EnvironmentNonvolatile& _envNV)
  {
    envNV = &_envNV;
  };

  EvalResult_t calculateFitness(const EnvironmentVolatile& env, size_t iTeam);
};

#endif /* SIMPLE_EVALUATOR_H */
