#ifndef PLANNER_HUMAN_H
#define PLANNER_HUMAN_H

#include "../inc/planner.h"

class PokemonAI;
class PkCU;

class EnvironmentNonvolatile;
union EnvironmentVolatile;

class planner_human : public Planner
{
private:
  static const std::string ident;

  PkCU* cu;

  size_t agentTeam;

  /* Returns a valid action as per the user's choice 
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   */
  unsigned int actionSelect(const EnvironmentVolatile& env);

  /*
   * Prints all possible actions a given pokemon may take to stdout
   */
  void printActions(const EnvironmentVolatile& env);

public:
  planner_human();

  planner_human(const planner_human& other);
  
  ~planner_human() { };

  planner_human* clone() const { return new planner_human(*this); }

  bool isInitialized() const;

  const std::string& getName() const { return ident; };

  void setEvaluator(const Evaluator& evalType) { /* we're not going to use the evaluator, so do nothing with it */ };
  const Evaluator* getEvaluator() const { return NULL; }

  void setEnvironment(PkCU& _cu, size_t _agentTeam);

  uint32_t generateSolution(const EnvironmentPossible& origin);
};

#endif /* PLANNER_HUMAN_H */
