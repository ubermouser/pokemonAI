#ifndef PLANNER_MAX_H
#define PLANNER_MAX_H

#include "../inc/pkai.h"
#include "../inc/planner.h"

class Evaluator;
class PkCU;

class PlannerMax : public Planner
{
protected:
  std::vector<PlannerResult> results;

public:
  PlannerMax(size_t agentTeam) : Planner("max_planner-NULLEVAL", agentTeam) {};
  PlannerMax(const PlannerMax& other) = default;
  
  ~PlannerMax() {};

  virtual std::string baseName() const override { return "max_planner"; }

  PlannerMax* clone() const override { return new PlannerMax(*this); }

  void setEngine(const std::shared_ptr<PkCU>& cu) override;

  uint32_t generateSolution(const ConstEnvironmentPossible& origin) override;

  static uint32_t generateSolution(
      PkCU& cu,
      Evaluator& eval,
      const ConstEnvironmentPossible& env,
      size_t _agentTeam,
      size_t* nodesEvaluated = NULL,
      std::vector<PlannerResult>* results = NULL);
};

#endif /* PLANNER_MAX_H */
