#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "../inc/pkai.h"

#include "../inc/name.h"

class EnvironmentNonvolatile;
class TeamNonVolatile;
class PokemonNonVolatile;
union EnvironmentVolatile;
union TeamVolatile;
union PokemonVolatile;

struct EvalResult_t
{
  fpType fitness;
  int8_t agentMove;
  int8_t otherMove;
};

class Evaluator: public HasName
{
public:
  /* delete evaluator */
  virtual ~Evaluator() { };

  /* returns a NEW copy of the current evaluator */
  virtual Evaluator* clone() const = 0;

  /* does the evaluator have all acceptable data required to perform evaluation? */
  virtual bool isInitialized() const { return true; };

  /* reset the nonvolatile team held by the evaluator. envNV is guaranteed to remain until next reset */
  virtual void resetEvaluator(const EnvironmentNonvolatile& envNV) { };

  /* evaluate the fitness of a given environment for team iTeam */
  virtual EvalResult_t calculateFitness(const EnvironmentVolatile& env, size_t iTeam) = 0;
};

#endif /* EVALUATOR_H */
