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

#include "ranked.h"
#include "planner.h"

class RankedPlanner : public Ranked {
public:
  RankedPlanner(
    std::shared_ptr<Planner>& planner);

  const std::string& getName() const { return planner_->getName(); }

  const Planner& get() const { return *planner_; };

  std::ostream& print(std::ostream& os) const;

protected:
  std::shared_ptr<Planner> planner_;
};


using RankedPlannerPtr = std::shared_ptr<RankedPlanner>;
using PlannerLeague = std::unordered_map<RankedPlanner::Hash, RankedPlannerPtr >;

std::ostream& operator <<(std::ostream& os, const RankedPlanner& tR);

#endif /* RANKED_PLANNER_H */

