#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include "planner.h"

class PlannerRandom : public Planner {
public:
  using base_t = Planner;
  struct Config: public Planner::Config {
    double moveChance = 0.75;

    Config() : Planner::Config() {};

    virtual boost::program_options::options_description options(
        const std::string& category="agent options", std::string prefix="") override;
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
