#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "pkai.h"

#include <memory>

#include "name.h"
#include "environment_volatile.h"
#include "environment_possible.h"

struct EvalResult_t {
  fpType fitness;
  Action agentMove;
  Action otherMove;
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

  virtual Evaluator& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv) {
    nv_ = nv;
    return *this;
  }

  /* does the evaluator have all acceptable data required to perform evaluation? */
  virtual bool isInitialized() const;

  /* evaluate the fitness of a given environment for team iTeam */
  virtual EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const = 0;
  virtual EvalResult_t calculateFitness(const ConstEnvironmentPossible& env, size_t iTeam) const {
    return calculateFitness(env.getEnv(), iTeam);
  }
};

#endif /* EVALUATOR_H */
