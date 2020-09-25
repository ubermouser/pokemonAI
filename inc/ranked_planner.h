/* 
 * File:   ranked_planner.h
 * Author: drendleman
 *
 * Created on September 23, 2020, 8:32 PM
 */

#ifndef RANKED_PLANNER_H
#define RANKED_PLANNER_H

#include "pkai.h"

#include <memory>
#include <unordered_map>

#include "ranked.h"
#include "planner.h"

class RankedPlanner : public Ranked {
public:
  RankedPlanner(const std::shared_ptr<Planner>& planner): Ranked(), planner_(planner) { identify(); }

  const std::string& getName() const override { return planner_->getName(); }

  const Planner& get() const { return *planner_; };

protected:
  virtual Hash generateHash(bool generateSubHashes = true) override;
  virtual std::string defineName() override { return planner_->getName(); };

  std::shared_ptr<Planner> planner_;
};


using RankedPlannerPtr = std::shared_ptr<RankedPlanner>;
using PlannerLeague = std::unordered_map<RankedPlanner::Hash, RankedPlannerPtr >;

#endif /* RANKED_PLANNER_H */

