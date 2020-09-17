#include "../inc/order_heuristic.h"

#include <algorithm>
#include <unordered_map>


void OrderHeuristic::initialize() {
  major_counts_.fill(ActionMap{});
}


size_t OrderHeuristic::getBin(const ConstEnvironmentVolatile& env, size_t iTeam) const {
  return (iTeam * 36) + (env.getTeam(iTeam).getICPKV() * 6) + env.getOtherTeam(iTeam).getICPKV();
}

void OrderHeuristic::increment(
    const ConstEnvironmentVolatile& env, size_t iTeam, const Action& action) {
  major_counts_[getBin(env, iTeam)][action] += 1;
}


ActionVector& OrderHeuristic::order(
    const ConstEnvironmentVolatile& env, size_t iTeam, ActionVector& actions, const Action& killer) const {

  auto getCount = [&](const Action& a){
    uint64_t count = 0;
    const ActionMap& counts = major_counts_[getBin(env, iTeam)];
    auto countIterator = counts.find(a);
    if (countIterator != counts.end()) {
      count = countIterator->second;
    }
    return count;
  };

  // sort actions in the ActionVector in order of their cutoff counts
  std::sort(std::begin(actions), std::end(actions), [&](auto& a, auto& b){
    if (a == killer) { return true; }
    else if (b == killer) { return false; }
    else {
      auto count_a = getCount(a);
      auto count_b = getCount(b);
      return count_a > count_b;
    }
  });

  return actions;
}