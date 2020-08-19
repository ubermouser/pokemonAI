#ifndef PLANNER_H
#define PLANNER_H

#include "pkai.h"

#include <memory>
#include <vector>

#include "environment_nonvolatile.h"
#include "environment_possible.h"
#include "evaluator.h"
#include "pkCU.h"
#include "name.h"


class PlannerResult
{
public:
  size_t depth;
  int bestAgentAction;
  int bestOtherAction;
  fpType lbFitness;
  fpType ubFitness;

  PlannerResult(size_t _depth, int _bestAgentAction, int _bestOtherAction, fpType _lbFitness, fpType _ubFitness)
    : depth(_depth),
    bestAgentAction(_bestAgentAction),
    bestOtherAction(_bestOtherAction),
    lbFitness(_lbFitness),
    ubFitness(_ubFitness)
  {
  };
};

class Planner : public Name {
protected:
  std::shared_ptr<PkCU> cu_;
  std::shared_ptr<Evaluator> eval_;
  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  std::vector<PlannerResult> results_;

  size_t agentTeam_;

  virtual std::string baseName() const { return "planner"; }
  virtual void resetName();

public:
  Planner() = delete;
  Planner(const std::string& name, size_t agentTeam): Name(name), agentTeam_(agentTeam) {};
  Planner(const Planner& other) = default;
  virtual ~Planner() { };

  /* create a copy of the current planner (and its evaluator) */
  virtual Planner* clone() const = 0;

  /* does the planner have all acceptable data required to perform planning? */
  virtual bool isInitialized() const;

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

  /* generate an action */
  virtual uint32_t generateSolution(const ConstEnvironmentPossible& origin) = 0;

  virtual const std::vector<PlannerResult>& getDetailedResults() const { return results_; };
  virtual void clearResults() { results_.clear(); };
};

#endif /* PLANNER_H */
