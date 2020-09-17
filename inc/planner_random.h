#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include "planner.h"

class PlannerRandom : public Planner {
public:
  struct Config: public Planner::Config {
    double moveChance = 0.7;

    Config() : Planner::Config() {};
  };

  PlannerRandom(const Config& cfg = Config()) : Planner(cfg, ident), cfg_(dynamic_cast<const Config&>(cfg)) {};

  PlannerRandom(const PlannerRandom& other) = default;
  virtual ~PlannerRandom() { };

  virtual PlannerRandom* clone() const override { return new PlannerRandom(*this); }

  virtual bool isEvaluatorRequired() const override { return false; }
  virtual size_t maxImplDepth() const override { return 0; }

  virtual const std::string& getName() const override { return ident; };

  virtual PlyResult generateSolutionAtLeaf(
    const ConstEnvironmentPossible& origin) const override;

protected:
  Config cfg_;

  static const std::string ident;
};

#endif /* PLANNER_RANDOM_H */
