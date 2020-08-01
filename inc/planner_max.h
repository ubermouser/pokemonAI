#ifndef PLANNER_MAX_H
#define PLANNER_MAX_H

#include "../inc/pkai.h"
#include "../inc/planner.h"

class Evaluator;
class PkCU;

class planner_max : public Planner
{
private:
  std::string ident;

  std::vector<PlannerResult> results;

  PkCU* cu;

  /* evaluator being used by this planner */
  Evaluator* eval;

  size_t agentTeam;

  size_t engineAccuracy;

public:
  planner_max(size_t engineAccuracy = 1);
  planner_max(const Evaluator& evalType, size_t engineAccuracy = 1);

  planner_max(const planner_max& other);
  
  ~planner_max();

  planner_max* clone() const { return new planner_max(*this); }

  bool isInitialized() const;

  const std::string& getName() const { return ident; };

  void setEvaluator(const Evaluator& evalType);
  const Evaluator* getEvaluator() const { return eval; }

  void setEnvironment(PkCU& _cu, size_t _agentTeam);

  uint32_t generateSolution(const EnvironmentPossible& origin);

  static uint32_t generateSolution(PkCU& _cu, Evaluator& eval, const EnvironmentPossible& origin, size_t _agentTeam, size_t* nodesEvaluated = NULL, std::vector<PlannerResult>* results = NULL);

  const std::vector<PlannerResult>& getDetailedResults() const { return results; };
  void clearResults() { results.clear(); };
};

#endif /* PLANNER_MAX_H */
