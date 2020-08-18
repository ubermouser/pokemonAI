#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "pkai.h"

#include <memory>

#include "name.h"
#include "environment_volatile.h"

struct EvalResult_t
{
  fpType fitness;
  int8_t agentMove;
  int8_t otherMove;
};

class Evaluator: public Name
{
protected:
  std::shared_ptr<const EnvironmentNonvolatile> nv_;

public:
  Evaluator() = default;
  Evaluator(const std::string& name): Name(name) {}

  /* delete evaluator */
  virtual ~Evaluator() { };

  /* returns a NEW copy of the current evaluator */
  virtual Evaluator* clone() const = 0;

  virtual void setEnvironment(std::shared_ptr<const EnvironmentNonvolatile>& nv) { nv_ = nv; }

  /* does the evaluator have all acceptable data required to perform evaluation? */
  virtual bool isInitialized() const { return true; };

  /* reset the nonvolatile team held by the evaluator. envNV is guaranteed to remain until next reset */
  virtual void resetEvaluator(std::shared_ptr<const EnvironmentNonvolatile>& envNV) { nv_ = envNV; };

  /* evaluate the fitness of a given environment for team iTeam */
  virtual EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) = 0;
  virtual EvalResult_t calculateFitness(const EnvironmentVolatileData& env, size_t iTeam) {
    return calculateFitness(ConstEnvironmentVolatile{*nv_, env}, iTeam);
  }
};

#endif /* EVALUATOR_H */
