#ifndef RANDOM_EVALUATOR_H
#define RANDOM_EVALUATOR_H

#include "../inc/evaluator.h"

class evaluator_random: public Evaluator
{
private:
  static const std::string ident;
public:

  ~evaluator_random() { };
  evaluator_random(): Evaluator("Random_Evaluator") { };

  evaluator_random* clone() const { return new evaluator_random(*this); }

  EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam);
};

#endif /* RANDOM_EVALUATOR_H */
