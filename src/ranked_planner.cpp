#include "../inc/ranked_planner.h"

#include <functional>


RankedPlanner::Hash RankedPlanner::generateHash(bool generateSubHashes) {
  hash_ = std::hash<Planner*>()(planner_.get());
  return hash_;
}
