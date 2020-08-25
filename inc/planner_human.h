#ifndef PLANNER_HUMAN_H
#define PLANNER_HUMAN_H

#include "planner.h"

class PlannerHuman : public Planner {
public:
  PlannerHuman(const Config& cfg = Config()) : Planner(cfg, ident) {};
  PlannerHuman(const PlannerHuman& other) = default;
  ~PlannerHuman() { };

  virtual PlannerHuman* clone() const override { return new PlannerHuman(*this); }

  virtual size_t maxImplDepth() const override { return 0; }
  virtual bool isEvaluatorRequired() const override { return false; }

  virtual PlyResult generateSolutionAtLeaf(
      const ConstEnvironmentVolatile& origin) const override;
protected:
  static const std::string ident;

  /* Returns a valid action as per the user's choice
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   */
  int32_t actionSelect(const ConstEnvironmentVolatile& env) const;

  /*
   * Prints all possible actions a given pokemon may take to stdout
   */
  void printActions(const ConstEnvironmentVolatile& env) const;

  virtual std::string baseName() const override { return "HumanPlanner"; }
};

#endif /* PLANNER_HUMAN_H */
