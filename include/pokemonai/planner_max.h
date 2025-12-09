#ifndef PLANNER_MAX_H
#define PLANNER_MAX_H

#include "pkai.h"
#include "planner.h"

class PlannerMax : public Planner {
public:
  PlannerMax(const Config& cfg = Config()) : Planner(cfg) { resetName(); };
  PlannerMax(const PlannerMax& other) = default;
  
  virtual ~PlannerMax() {};

  virtual PlannerMax* clone() const override { return new PlannerMax(*this); }

  virtual PlannerMax& setEngine(const std::shared_ptr<PkCU>& cu) override;
  virtual PlannerMax& setEngine(const PkCU& cu) override {
    // TODO(@drendleman) why do we need to override this?
    return setEngine(std::make_shared<PkCU>(cu));
  }

protected:
  virtual std::string baseName() const override { return "Max"; }

  virtual size_t maxImplDepth() const override { return MAXTRIES; }

  virtual PlyResult generateSolutionAtDepth(
      const ConstEnvironmentPossible& origin, size_t maxPly) const override;
};

#endif /* PLANNER_MAX_H */
