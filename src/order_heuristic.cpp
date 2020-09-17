#include "../inc/order_heuristic.h"

#include <algorithm>
#include <unordered_map>


void OrderHeuristic::initialize() {
  major_counts_.fill(ActionMap{});
}

void OrderHeuristic::increment(
    const ConstEnvironmentVolatile& env, size_t iTeam, const Action& action) {
  major_counts_[iTeam][action] += 1;
}


ActionVector& OrderHeuristic::order(
    const ConstEnvironmentVolatile& env, size_t iTeam, ActionVector& actions) const {

  auto getCount = [&](const Action& a){
    uint64_t count = 0;
    auto countIterator = major_counts_[iTeam].find(a);
    if (countIterator != major_counts_[iTeam].end()) {
      count = countIterator->second;
    }
    return count;
  };

  // TODO(@drendleman) - why doesn't this sort?
  // sort actions in the ActionVector in order of their cutoff counts
  std::sort(std::begin(actions), std::end(actions), [&](auto& a, auto& b){
    auto count_a = getCount(a);
    auto count_b = getCount(b);
    return count_a > count_b;
  });

  return actions;
}