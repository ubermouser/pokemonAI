#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include "planner.h"

class PlannerRandom : public Planner {
public:
  using base_t = Planner;
  struct Config: public Planner::Config {
    double moveChance = 0.75;

    Config() : Planner::Config() {maxDepth = 0;};

    virtual boost::program_options::options_description options(
        const std::string& category="agent options", std::string prefix="") override;
  };

  PlannerRandom(
      const Config& cfg = Config())
    : Planner(cfg),
      cfg_(dynamic_cast<const Config&>(cfg)) {
    resetName();
  };

  PlannerRandom(const PlannerRandom& other) = default;
  virtual ~PlannerRandom() { };

  virtual PlannerRandom* clone() const override { return new PlannerRandom(*this); }

protected:
  virtual bool isEvaluatorRequired() const override { return false; }
  virtual size_t maxImplDepth() const override { return 0; }

  virtual std::string baseName() const override { return "Random"; }
  virtual void resetName() override;

  virtual PlyResult generateSolutionAtLeaf(
    const ConstEnvironmentPossible& origin) const override;

protected:
  Config cfg_;
};

#endif /* PLANNER_RANDOM_H */
