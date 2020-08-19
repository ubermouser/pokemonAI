#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include "planner.h"

class PlannerRandom : public Planner {
private:
  static const std::string ident;
public:
  PlannerRandom(size_t agentTeam=SIZE_MAX) : Planner(ident, agentTeam) {};

  PlannerRandom(const PlannerRandom& other) = default;
  virtual ~PlannerRandom() { };

  virtual PlannerRandom* clone() const override { return new PlannerRandom(*this); }

  virtual bool isInitialized() const override;

  virtual const std::string& getName() const override { return ident; };

  uint32_t generateSolution(const ConstEnvironmentPossible& origin) override;
};

#endif /* PLANNER_RANDOM_H */
