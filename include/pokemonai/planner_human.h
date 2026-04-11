#ifndef PLANNER_HUMAN_H
#define PLANNER_HUMAN_H

#include "planner.h"

#include <iosfwd>
#include <functional>

class PlannerHuman : public Planner {
public:
  PlannerHuman(const Config& cfg = Config());
  PlannerHuman(
      const Config& cfg,
      std::istream& input)
    : Planner(cfg),
      in_(input) {
    resetName();
  };
  PlannerHuman(const PlannerHuman& other) = default;
  ~PlannerHuman() { };

  virtual PlannerHuman* clone() const override { return new PlannerHuman(*this); }

protected:
  std::reference_wrapper<std::istream> in_;

  bool shouldEval() const { return eval_ != nullptr; }

  /* Returns a valid ActionMap as per the user's choice
   */
  ActionMap actionSelect(const ConstEnvironmentVolatile& env) const;

  /*
   * Prints all possible actions a given pokemon may take to stdout
   */
  void printActions(const ConstEnvironmentPossible& env) const;

  virtual std::string baseName() const override { return "Human"; }

  virtual size_t maxImplDepth() const override { return 0; }
  virtual bool isEvaluatorRequired() const override { return false; }

  virtual PlyResult generateSolutionAtLeaf(
      const ConstEnvironmentPossible& origin) const override;
};

#endif /* PLANNER_HUMAN_H */
