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

  virtual PlannerHuman& setEngine(const std::shared_ptr<PkCU>& cu) override;
  virtual PlannerHuman& setEngine(const PkCU& cu) override {
    return setEngine(std::make_shared<PkCU>(cu));
  }

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

  /**
   * Collects actions for a given actor, grouped by move type
   */
  std::unordered_map<size_t, ActionVector> actionsPerMoveType(
      const ConstEnvironmentVolatile& env, const Actor& actor) const;

  /**
   * Prints all possible move actions for a given team
   */
  void printActions_moves(
      const ConstEnvironmentPossible& env,
      const ConstTeamVolatile& cTeam,
      const std::vector<Actor>& actors,
      double currentFitness) const;

  /**
   * Prints all possible swap actions for a given team
   */
  void printActions_swaps(
      const ConstEnvironmentPossible& env,
      const ConstTeamVolatile& cTeam,
      const std::vector<Actor>& actors,
      double currentFitness) const;

  /**
   * Prints all possible new-teammate activations
   */
  void printActions_activations(
      const ConstEnvironmentPossible& env,
      const ConstTeamVolatile& cTeam,
      double currentFitness) const;

  /**
   * Calculates the change in fitness for a given action
   */
  std::string getFitnessDelta(
      const ConstEnvironmentPossible& env,
      const ConstTeamVolatile& cTeam,
      double currentFitness,
      const Actor& actor,
      const Action& action) const;

  virtual std::string baseName() const override { return "Human"; }

  virtual size_t maxImplDepth() const override { return 0; }
  virtual bool isEvaluatorRequired() const override { return false; }

  virtual PlyResult generateSolutionAtLeaf(
      const ConstEnvironmentPossible& origin) const override;
};

#endif /* PLANNER_HUMAN_H */
