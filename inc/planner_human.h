#ifndef PLANNER_HUMAN_H
#define PLANNER_HUMAN_H

#include "planner.h"

class PlannerHuman : public Planner {
protected:
  static const std::string ident;

  /* Returns a valid action as per the user's choice 
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   */
  unsigned int actionSelect(const ConstEnvironmentVolatile& env);

  /*
   * Prints all possible actions a given pokemon may take to stdout
   */
  void printActions(const ConstEnvironmentVolatile& env);

  void resetName() override { setName(ident); }

public:
  PlannerHuman(size_t agentTeam=SIZE_MAX) : Planner(ident, agentTeam) {};
  PlannerHuman(const PlannerHuman& other) = default;
  ~PlannerHuman() { };

  PlannerHuman* clone() const override { return new PlannerHuman(*this); }

  virtual bool isInitialized() const override;

  const std::string& getName() const override { return ident; };

  uint32_t generateSolution(const ConstEnvironmentPossible& origin) override;
};

#endif /* PLANNER_HUMAN_H */
