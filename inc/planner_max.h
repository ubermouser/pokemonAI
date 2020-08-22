#ifndef PLANNER_MAX_H
#define PLANNER_MAX_H

#include "pkai.h"
#include "planner.h"

class PlannerMax : public Planner {
public:
  PlannerMax(const Planner::Config& cfg = Planner::Config()) : Planner(cfg) {};
  PlannerMax(const PlannerMax& other) = default;
  
  virtual ~PlannerMax() {};

  virtual std::string baseName() const override { return "MaxPlanner"; }

  virtual PlannerMax* clone() const override { return new PlannerMax(*this); }

  virtual PlannerMax& setEngine(const std::shared_ptr<PkCU>& cu) override;
  virtual PlannerMax& setEngine(const PkCU& cu) override {
    // TODO(@drendleman) why do we need to override this?
    return setEngine(std::make_shared<PkCU>(cu));
  }

  virtual size_t maxImplDepth() const override { return 1; }

  virtual PlyResult generateSolutionAtDepth(
      const ConstEnvironmentVolatile& origin, size_t maxPly) const override;
};

#endif /* PLANNER_MAX_H */
