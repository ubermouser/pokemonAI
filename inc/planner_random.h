#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include "planner.h"

class planner_random : public Planner {
private:
  static const std::string ident;
public:
  planner_random(size_t agentTeam) : Planner(ident, agentTeam) {};

  planner_random(const planner_random& other) = default;
  virtual ~planner_random() { };

  virtual planner_random* clone() const override { return new planner_random(*this); }

  virtual bool isInitialized() const override;

  virtual const std::string& getName() const override { return ident; };

  uint32_t generateSolution(const ConstEnvironmentPossible& origin) override;
};

#endif /* PLANNER_RANDOM_H */
