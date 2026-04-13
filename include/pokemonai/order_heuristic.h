/* 
 * File:   order_heuristic.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:37 PM
 */

#ifndef ORDERHEURISTIC_H
#define	ORDERHEURISTIC_H

#include <array>
#include <unordered_map>
#include <vector>

#include "action.h"
#include "actor.h"
#include "environment_volatile.h"

class OrderHeuristic {
 public:
  struct PairHash {
    size_t operator()(const std::pair<Actor, Action>& p) const {
      auto h1 = std::hash<Actor>{}(p.first);
      auto h2 = std::hash<Action>{}(p.second);
      return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
  };

  using ActionCounts =
      std::unordered_map<std::pair<Actor, Action>, uint64_t, PairHash>;

  void increment(const Actor& actor, const Action& action);
  void increment(const ActionMap& actionMap);

  std::vector<ActionMap>& order(
      const ConstEnvironmentVolatile& env,
      std::vector<ActionMap>& actions,
      const ActionMap& killer = ActionMap{}) const;

  void initialize();

 protected:
  ActionCounts counts_;
};

#endif
