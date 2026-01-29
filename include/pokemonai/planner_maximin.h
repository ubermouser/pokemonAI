/*
 * File:   planner_maximin.h
 * Author: ubermouser
 *
 * Created on August 21, 2020, 7:49 PM
 */

#ifndef PLANNER_MAXIMIN_H
#define PLANNER_MAXIMIN_H

#include "pkai.h"
#include "planner.h"

/**
 * @class PlannerMaximin
 * @brief A planner implementing depth-first-search over leaf evaluation.
 *
 * No search optimization is implemented at all for this planner, offering a
 * stable baseline of performance versus more efficient pruning methods.
 */
class PlannerMaximin : public Planner {
 public:
 PlannerMaximin(const Config& cfg = Config()) : Planner(cfg) { resetName(); };
 PlannerMaximin(const PlannerMaximin& other) = default;

 virtual ~PlannerMaximin(){};

 virtual std::string baseName() const override { return "Maximin"; }

 virtual PlannerMaximin* clone() const override {
   return new PlannerMaximin(*this);
 }

  virtual size_t maxImplDepth() const override { return MAXTRIES; }

  virtual PlyResult generateSolutionAtDepth(
      const ConstEnvironmentPossible& origin, size_t maxPly) const override;
};

#endif /* PLANNER_MAXIMIN_H */

