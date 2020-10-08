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
#include "pkCU.h"


struct FitnessDepth {
  Fitness fitness;
  uint32_t depth;

  FitnessDepth(const Fitness& fitness_ = Fitness::worst(), const size_t depth_ = 0);

  bool fullyEvaluated() const { return depth >= MAXTRIES; }

  static FitnessDepth worst() { return FitnessDepth{Fitness::worst(), MAXTRIES}; }
  static FitnessDepth best() { return FitnessDepth{Fitness::best(), MAXTRIES}; }

  FitnessDepth expand(const Fitness::precision_t& probability) const {
    return FitnessDepth{fitness.expand(probability), depth};
  }

  bool operator < (const FitnessDepth& other) const {
    // bias towards shallower depth
    if (fitness == other.fitness) {
      return depth < other.depth;
    }
    return fitness < other.fitness;
  }
  bool operator > (const FitnessDepth& other) const {
    if (fitness == other.fitness) {
      return depth < other.depth;
    }
    return fitness > other.fitness;
  }
};


struct EvalResult : public FitnessDepth {
  Action agentAction;
  Action otherAction;

  EvalResult(
      const FitnessDepth& fitnessDepth_,
      const Action& agent = Action{},
      const Action& other = Action{}) :
      FitnessDepth(fitnessDepth_),
      agentAction(agent),
      otherAction(other) { }
  EvalResult(
      const Fitness& fitness_ = Fitness::worst(),
      const Action& agent = Action{},
      const Action& other = Action{},
      const size_t depth_ = 0) :
      FitnessDepth(fitness_, depth_),
      agentAction(agent),
      otherAction(other) { }
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

  virtual Evaluator& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& env);
  virtual Evaluator& setEnvironment(const EnvironmentNonvolatile& nv) {
    return setEnvironment(std::make_shared<EnvironmentNonvolatile>(nv));
  }

  virtual Evaluator& setEngine(const std::shared_ptr<PkCU>& cu);
  virtual Evaluator& setEngine(const PkCU& cu) {
    return setEngine(std::make_shared<PkCU>(cu));
  }

  /* does the evaluator have all acceptable data required to perform evaluation? */
  virtual Evaluator& initialize();

  /* evaluate the fitness of a given environment for team iTeam */
  virtual EvalResult evaluate(const ConstEnvironmentVolatile& env, size_t iTeam) const;
  virtual EvalResult evaluate(const ConstEnvironmentPossible& env, size_t iTeam) const {
    return evaluate(env.getEnv(), iTeam);
  }
protected:
  Config cfg_;

  /* nonvolatile environment description */
  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  /* state transition engine */
  std::shared_ptr<PkCU> cu_;

  virtual EvalResult calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const = 0;

  virtual Fitness combineTeamFitness(fpType agentFitness, fpType otherFitness) const;

  virtual std::string baseName() const { return "Evaluator"; }
  virtual void resetName();
};

#endif /* EVALUATOR_H */
