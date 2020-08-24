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

class PlannerMaxiMin : public Planner {
public:
  PlannerMaxiMin(const Planner::Config& cfg = Planner::Config()) : Planner(cfg) {};
  PlannerMaxiMin(const PlannerMaxiMin& other) = default;

  virtual ~PlannerMaxiMin() {};

  virtual std::string baseName() const override { return "MaxiMinPlanner"; }

  virtual PlannerMaxiMin* clone() const override { return new PlannerMaxiMin(*this); }

  virtual size_t maxImplDepth() const override { return MAXTRIES; }

  virtual PlyResult generateSolutionAtDepth(
      const ConstEnvironmentVolatile& origin, size_t maxPly) const override;
};

#endif /* PLANNER_MAXIMIN_H */

