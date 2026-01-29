#ifndef PLANNER_MINIMAX_H
#define PLANNER_MINIMAX_H

#include "planner_maximin.h"

/**
 * @class PlannerMinimax
 * @brief A planner implementing expectiminimax search over leaf evaluation.
 *
 * Includes alpha-beta-gamma pruning.
 */
class PlannerMinimax : public PlannerMaximin {
public:
  PlannerMinimax(const Config& cfg = Config()) : PlannerMaximin(cfg) { resetName(); };
  PlannerMinimax(const PlannerMinimax& other) = default;

  virtual ~PlannerMinimax() {};

  virtual std::string baseName() const override { return "Minimax"; }

  virtual PlannerMinimax* clone() const override { return new PlannerMinimax(*this); }

protected:
  /* test if another previously seen action always better than the current */
  virtual bool testGammaCutoff(
      const EvalResult& child,
      const FitnessDepth& lowCutoff,
      const FitnessDepth& highCutoff) const override;

  /* test if the current agent action is better than the best seen agent action */
  virtual bool testAgentSelection(
      EvalResult& bestOfWorst, 
      const EvalResult& worst,
      const FitnessDepth& lowCutoff,
      const ConstEnvironmentPossible& origin) const override;

  /* test if the current other action is worse than the worst seen other action */
  virtual bool testOtherSelection(
      EvalResult& worst, 
      const EvalResult& current,
      const FitnessDepth& highCutoff,
      const ConstEnvironmentPossible& origin) const override;
};

#endif /* PLANNER_MINIMAX_H */
