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

  size_t engineAccuracy_;

public:
  PlannerMax(size_t agentTeam, size_t engineAccuracy = 1);
  PlannerMax(const PlannerMax& other) = default;
  
  ~PlannerMax() {};

  virtual std::string baseName() const override { return "max_planner"; }

  PlannerMax* clone() const { return new PlannerMax(*this); }

  void setEngine(std::shared_ptr<PkCU>& cu) override;

  uint32_t generateSolution(const EnvironmentPossible& origin);

  static uint32_t generateSolution(
      PkCU& cu,
      Evaluator& eval,
      const ConstEnvironmentPossible& env,
      size_t _agentTeam,
      size_t* nodesEvaluated = NULL,
      std::vector<PlannerResult>* results = NULL);

  const std::vector<PlannerResult>& getDetailedResults() const { return results; };
  void clearResults() { results.clear(); };
};

#endif /* PLANNER_MAX_H */
