/* 
 * File:   planner_multithreaded.h
 * Author: drendleman
 *
 * Created on August 25, 2020, 3:45 PM
 */

#ifndef PLANNER_MULTITHREADED_H
#define PLANNER_MULTITHREADED_H

#include "planner.h"

#include <memory>
#include <thread>
#include <vector>


class PlannerMultithreaded : public Planner {
public:
  struct Config : public Planner::Config {
    size_t numThreads;

    Config() : Planner::Config() {}
  };

  virtual PlannerMultithreaded& initialize();

  PlannerMultithreaded(const Config& cfg = Config()): Planner(cfg): cfg_(cfg) {};
protected:
  Config cfg_;

  mutable std::vector<std::thread> threadPool_;

  void instantiateThreads();

  void waitThreads();

  void haltThreads();
};

#endif /* PLANNER_MULTITHREADED_H */

