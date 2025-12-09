#ifndef PLANNER_STOCHASTIC_H
#define PLANNER_STOCHASTIC_H

#include "pokemonai/pkai.h"
#include "pokemonai/planner.h"

class Evaluator;
class PkCU;

class planner_stochastic : public Planner
{
private:
  std::string ident;

  std::vector<PlannerResult> results;

  PkCU* cu;

  /* evaluator being used by this planner */
  Evaluator* eval;

  size_t agentTeam;

  size_t engineAccuracy;

  /* A parameter that tunes how likely the planner will choose an action with better fitness compared to
   * an action with lower fitness when performing exploration. As temperature approaches zero, the odds 
   * of selecting a less maximized fitness are reduced. */
  fpType temperature;

  /* probability that the planner will explore the state space instead of choosing
   * the action that maximizes fitness. 1 implies full stochasticism, 0 implies full determinism */
  fpType exploration;

public:
  planner_stochastic(size_t _engineAccuracy = 1, fpType _temperature = 1.0, fpType _exploration = 0.5);
  planner_stochastic(const Evaluator& _eval, size_t _engineAccuracy = 1, fpType _temperature = 1.0, fpType _exploration = 0.5);

  planner_stochastic(const planner_stochastic& other);
  
  ~planner_stochastic();

  planner_stochastic* clone() const { return new planner_stochastic(*this); }

  bool isInitialized() const;

  const std::string& getName() const { return ident; };

  void setEvaluator(const Evaluator& evalType);
  const Evaluator* getEvaluator() const { return eval; }

  void setEnvironment(PkCU& _cu, size_t _agentTeam);

  uint32_t generateSolution(const EnvironmentPossible& origin);

  const std::vector<PlannerResult>& getDetailedResults() const { return results; };
  void clearResults() { results.clear(); };
};

#endif /* PLANNER_STOCHASTIC_H */
