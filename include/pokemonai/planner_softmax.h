/*
 * File:   planner_softmax.h
 * Author: ubermouser
 *
 * Created on January 23, 2026, 12:35 PM
 */

#ifndef PLANNER_SOFTMAX_H
#define PLANNER_SOFTMAX_H

#include "pkai.h"
#include "planner_maximin.h"

/**
 * @class PlannerSoftmax
 * @brief A planner implementing softmax selection over minimax leaf evaluations.
 */
class PlannerSoftmax : public PlannerMaxiMin {
public:
  using base_t = PlannerMaxiMin;

  struct Config : public base_t::Config {
    /* Temperature parameter for Boltzmann distribution. 
     * Low values (T->0) lead to more greedy, deterministic selection.
     * High values (T->inf) lead to more uniform, random selection. */
    double temperature = 1.0;

    Config() {};
    virtual ~Config() {};

    virtual boost::program_options::options_description options(
        const std::string& category="agent options", std::string prefix="") override;
  };

  PlannerSoftmax(const Config& cfg = Config()) : PlannerMaxiMin(cfg), cfg_(cfg) { resetName(); };
  PlannerSoftmax(const PlannerSoftmax& other) = default;

  virtual ~PlannerSoftmax() {};

  virtual std::string baseName() const override { return "Softmax"; }

  virtual PlannerSoftmax* clone() const override { return new PlannerSoftmax(*this); }

  virtual PlyResult generateSolutionAtDepth(
      const ConstEnvironmentPossible& origin, size_t maxPly) const override;

protected:
  Config cfg_;

  virtual void resetName() override;

  size_t selectAction(const std::vector<EvalResult>& actionValues) const;
};

#endif /* PLANNER_SOFTMAX_H */
