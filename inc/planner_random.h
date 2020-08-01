#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include "../inc/planner.h"

class PokemonAI;
class PkCU;

class planner_random : public Planner
{
private:
  static const std::string ident;

  PkCU* cu;

  size_t agentTeam;

public:
  planner_random();

  planner_random(const planner_random& other);
  
  ~planner_random() { };

  planner_random* clone() const { return new planner_random(*this); }

  bool isInitialized() const;

  const std::string& getName() const { return ident; };

  void setEvaluator(const Evaluator& evalType) { /* we're not going to use the evaluator, so do nothing with it */ };
  const Evaluator* getEvaluator() const { return NULL; }

  void setEnvironment(PkCU& _cu, size_t _agentTeam);

  uint32_t generateSolution(const EnvironmentPossible& origin);
};

#endif /* PLANNER_RANDOM_H */
