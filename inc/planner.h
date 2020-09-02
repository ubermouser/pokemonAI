#ifndef PLANNER_H
#define PLANNER_H

#include "pkai.h"

#include <memory>
#include <vector>

#include <boost/program_options.hpp>

#include "action.h"
#include "environment_nonvolatile.h"
#include "environment_possible.h"
#include "evaluator.h"
#include "fitness.h"
#include "pkCU.h"
#include "name.h"


struct PlyResult {
  size_t depth = 0;
  Action agentAction = Action{};
  Action otherAction = Action{};
  Fitness fitness = Fitness::worst();
  size_t numNodes = 0;
  double timeSpent = 0;
};

struct PlannerResult {
  std::vector<PlyResult> atDepth;

  bool hasSolution() const {
    return !atDepth.empty();
  }

  const PlyResult& best() const {
    return atDepth.back();
  }

  const Action& bestAgentAction() const {
    return best().agentAction;
  }

  const Action& bestOtherAction() const {
    return best().otherAction;
  }


};

class Planner : public Name {
public:
  struct Config {
    /* Maximum ply depth of computation. */
    size_t maxDepth = 0;

    /* Maximum amount of time to take per move in seconds. */
    double maxTime = 10.;

    /*
     * verbosity level, controls status printing.
     * 0: nothing is printed.
     * 1: terminal planner decision is printed.
     * 2: each completed ply decision is printed.
     */
    int verbosity = 0;

    Config() {};

    boost::program_options::options_description options(
        Config& cfg, const std::string& category="agent options", std::string prefix="");
  };

  Planner() = delete;
  Planner(
      const Config& cfg = Config(), const std::string& name = "Planner"):
      Name(name), cfg_(cfg), agentTeam_(SIZE_MAX) {};
  Planner(const Planner& other) = default;
  virtual ~Planner() { };

  /* create a copy of the current planner (and its evaluator) */
  virtual Planner* clone() const = 0;

  /* does the planner have all acceptable data required to perform planning? */
  virtual Planner& initialize();

  /* if the planner uses an evaluator, set the evaluator to evalType */
  virtual Planner& setEvaluator(const std::shared_ptr<Evaluator>& evaluator);
  virtual Planner& setEvaluator(const Evaluator& evaluator) {
    return setEvaluator(std::shared_ptr<Evaluator>(evaluator.clone()));
  }

  /* sets the nonvolatile environment to _cEnvironment, the team being referenced */
  virtual Planner& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& env);
  virtual Planner& setEnvironment(const EnvironmentNonvolatile& nv) {
    return setEnvironment(std::make_shared<EnvironmentNonvolatile>(nv));
  }

  virtual Planner& setEngine(const std::shared_ptr<PkCU>& cu);
  virtual Planner& setEngine(const PkCU& cu) {
    return setEngine(std::make_shared<PkCU>(cu));
  }

  virtual Planner& setTeam(size_t iTeam) { agentTeam_ = iTeam; return *this; };

  /* generate an action */
  virtual PlannerResult generateSolution(const ConstEnvironmentVolatile& origin) const;
  virtual PlannerResult generateSolution(const ConstEnvironmentPossible& origin) const {
    return generateSolution(origin.getEnv());
  }
protected:
  Config cfg_;

  /* state transition engine */
  std::shared_ptr<PkCU> cu_;

  /* state evaluator */
  std::shared_ptr<Evaluator> eval_;

  /* nonvolatile environment description */
  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  /* team that this agent represents */
  size_t agentTeam_;

  virtual size_t maxImplDepth() const { return 0; }
  virtual bool isEvaluatorRequired() const { return true; }

  virtual std::string baseName() const { return "Planner"; }
  virtual void resetName();
  
  /* generate a prediction at a specific depth */
  virtual PlyResult generateSolutionAtDepth(
      const ConstEnvironmentVolatile& origin, size_t maxPly) const;

  virtual PlyResult generateSolutionAtLeaf(const ConstEnvironmentVolatile& origin) const;

  /* Recurse through agent and other actions, pruning nodes above high and below low */
  virtual Fitness recurse_alphabeta(
      const ConstEnvironmentVolatile& origin,
      size_t iDepth,
      const Fitness& lowCutoff = Fitness::worst(),
      const Fitness& highCutoff = Fitness::best(),
      size_t* nodesEvaluated=NULL) const;

  /* Recurse through chance nodes, pruning nodes above high and below low*/
  virtual Fitness recurse_gamma(
      const ConstEnvironmentVolatile& origin,
      const Action& agentAction,
      const Action& otherAction,
      size_t iDepth,
      const Fitness& lowCutoff = Fitness::worst(),
      const Fitness& highCutoff = Fitness::best(),
      size_t* nodesEvaluated=NULL) const;

  /* generate all possible environments from a given origin, agent and other action pair. */
  virtual PossibleEnvironments generateStates(
      const ConstEnvironmentVolatile& origin,
      const Action& agentAction,
      const Action& otherAction) const;

  virtual void printSolution(const PlannerResult& result, bool isLast) const;
};

#endif /* PLANNER_H */
