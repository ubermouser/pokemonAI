#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include "planner.h"

class PlannerRandom : public Planner {
private:
  static const std::string ident;
public:
  PlannerRandom(const Config& cfg = Config()) : Planner(cfg, ident) {};

  PlannerRandom(const PlannerRandom& other) = default;
  virtual ~PlannerRandom() { };

  virtual PlannerRandom* clone() const override { return new PlannerRandom(*this); }

  virtual bool isEvaluatorRequired() const override { return false; }
  virtual size_t maxImplDepth() const override { return 1; }

  virtual const std::string& getName() const override { return ident; };

  PlyResult generateSolutionAtDepth(
    const ConstEnvironmentPossible& origin, size_t maxPly) const override;
};

#endif /* PLANNER_RANDOM_H */
