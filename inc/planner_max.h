#ifndef PLANNER_MAX_H
#define PLANNER_MAX_H

#include "pkai.h"
#include "planner.h"

class PlannerMax : public Planner {
public:
  PlannerMax(size_t agentTeam=SIZE_MAX) : Planner("max_planner-NULLEVAL", agentTeam) {};
  PlannerMax(const PlannerMax& other) = default;
  
  virtual ~PlannerMax() {};

  virtual std::string baseName() const override { return "max_planner"; }

  virtual PlannerMax* clone() const override { return new PlannerMax(*this); }

  virtual PlannerMax& setEngine(const std::shared_ptr<PkCU>& cu) override;

  virtual uint32_t generateSolution(const ConstEnvironmentPossible& origin) override;

  static uint32_t generateSolution(
      PkCU& cu,
      Evaluator& eval,
      const ConstEnvironmentPossible& env,
      size_t _agentTeam,
      size_t* nodesEvaluated = NULL,
      std::vector<PlannerResult>* results = NULL);
};

#endif /* PLANNER_MAX_H */
