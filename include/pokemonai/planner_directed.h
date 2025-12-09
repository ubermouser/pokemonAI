#ifndef PLANNER_DIRECTED_H
#define PLANNER_DIRECTED_H

#include "pokemonai/pkai.h"

#include "pokemonai/planner.h"
#include "pokemonai/experienceNet.h"

class Evaluator;
class experienceNet;
class evaluator_featureVector;

class planner_directed : public Planner
{
private:
  std::string ident;

  std::vector<PlannerResult> results;

  PkCU* cu;

  /* evaluator being used by this planner */
  evaluator_featureVector* eval;

  /* experience accumulator of eval */
  experienceNet exp;

  size_t agentTeam;

  size_t engineAccuracy;

  fpType bias;

  bool updateExperience;

public:
  planner_directed(
    const experienceNet& _exp = experienceNet(1), 
    size_t _engineAccuracy = 1, 
    fpType _bias = 0.5,
    bool _updateExperience = true);
  planner_directed(
    const evaluator_featureVector& _eval, 
    const experienceNet& _exp, 
    size_t _engineAccuracy = 1, 
    fpType _bias = 0.5,
    bool _updateExperience = true);

  planner_directed(const planner_directed& other);
  
  ~planner_directed();

  planner_directed* clone() const { return new planner_directed(*this); }

  bool isInitialized() const;

  const std::string& getName() const { return ident; };

  void setEvaluator(const Evaluator& evalType);
  const Evaluator* getEvaluator() const;

  void setEnvironment(PkCU& _cu, size_t _agentTeam);

  void setExperience(const experienceNet& _exp);
  const experienceNet& getExperience() const { return exp; };
  void clearExperience() { exp.clear(); };

  uint32_t generateSolution(const EnvironmentPossible& origin);

  const std::vector<PlannerResult>& getDetailedResults() const { return results; };
  void clearResults() { results.clear(); };
};

#endif /* PLANNER_DIRECTED_H */
