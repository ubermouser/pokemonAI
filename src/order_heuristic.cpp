#include "pokemonai/order_heuristic.h"

#include <algorithm>
#include <unordered_map>


void OrderHeuristic::initialize() { counts_.clear(); }


void OrderHeuristic::increment(const Actor& actor, const Action& action) {
  counts_[{actor, action}] += 1;
}


void OrderHeuristic::increment(const ActionMap& actionMap) {
  for (const auto& [actor, action] : actionMap) { increment(actor, action); }
}


std::vector<ActionMap>& OrderHeuristic::order(
    const ConstEnvironmentVolatile& env,
    std::vector<ActionMap>& actions,
    const ActionMap& killer) const {
  auto getMapCount = [&](const ActionMap& map) {
    uint64_t total = 0;
    for (const auto& [actor, action] : map) {
      auto it = counts_.find({actor, action});
      if (it != counts_.end()) { total += it->second; }
    }
    return total;
  };

  // sort actions in the ActionVector in order of their cutoff counts
  std::sort(
      std::begin(actions),
      std::end(actions),
      [&](const auto& a, const auto& b) {
        if (a == killer) {
          return true;
        } else if (b == killer) {
          return false;
        } else {
          auto count_a = getMapCount(a);
          auto count_b = getMapCount(b);
          return count_a > count_b;
        }
      });

  return actions;
}