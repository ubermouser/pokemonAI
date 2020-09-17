#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "pkai.h"

#include <memory>
#include <string>

#include <boost/program_options.hpp>

#include "action.h"
#include "name.h"
#include "environment_volatile.h"
#include "environment_possible.h"
#include "fitness.h"

struct EvalResult {
  Action agentAction;
  Action otherAction;
  uint32_t depth;
  Fitness fitness;

  EvalResult(
      const Fitness& fitness_ = Fitness::worst(),
      const Action& agent = Action{},
      const Action& other = Action{},
      const size_t depth_ = 0) :
      agentAction(agent),
      otherAction(other),
      depth(depth_),
      fitness(fitness_) { }
      
  bool operator < (const EvalResult& other) const { return fitness < other.fitness; }
  bool operator > (const EvalResult& other) const { return fitness > other.fitness; }
};

class Evaluator: public Name {
public:
  struct Config {
    /* how much more heavily we weight our own fitness compared to the enemy's fitness. */
    fpType teamBias = 0.5;

    Config() {};
    virtual ~Config() {};

    virtual boost::program_options::options_description options(
        const std::string& category="evaluator options", std::string prefix="");
  };

  Evaluator(const Config& cfg = Config()) : Name(), cfg_(cfg) {}
  Evaluator(const std::string& name, const Config& cfg = Config()): Name(name), cfg_(cfg) {}

  /* delete evaluator */
  virtual ~Evaluator() { };

  /* returns a NEW copy of the current evaluator */
  virtual Evaluator* clone() const = 0;

  virtual Evaluator& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv) {
    nv_ = nv;
    return *this;
  }

  /* does the evaluator have all acceptable data required to perform evaluation? */
  virtual Evaluator& initialize();

  /* evaluate the fitness of a given environment for team iTeam */
  virtual EvalResult calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const = 0;
  virtual EvalResult calculateFitness(const ConstEnvironmentPossible& env, size_t iTeam) const {
    return calculateFitness(env.getEnv(), iTeam);
  }
protected:
  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  Config cfg_;

  virtual Fitness combineTeamFitness(fpType agentFitness, fpType otherFitness) const;
};

#endif /* EVALUATOR_H */
