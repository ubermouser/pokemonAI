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

#include "action.h"
#include "environment_volatile.h"

class OrderHeuristic {
public:
  using ActionMap = std::unordered_map<Action, uint64_t>;

  void increment(const ConstEnvironmentVolatile& env, size_t iTeam, const Action& action);

  ActionVector& order(const ConstEnvironmentVolatile& env, size_t iTeam, ActionVector& actions) const;

  void initialize();
protected:
  std::array<ActionMap, 2> major_counts_;
};

#endif
