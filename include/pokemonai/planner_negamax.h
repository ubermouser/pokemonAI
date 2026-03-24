#ifndef PLANNER_NEGAMAX_H
#define PLANNER_NEGAMAX_H

#include <boost/program_options.hpp>
#include <string>

#include "order_heuristic.h"
#include "planner_minimax.h"
#include "transposition_table.h"

/**
 * @class PlannerNegamax
 * @brief A planner implementing negamax search over leaf evaluation.
 *
 * Includes alpha-beta-gamma pruning.
 */
class PlannerNegamax : public PlannerMinimax {
 public:
  using base_t = PlannerMinimax;

  struct Config : public base_t::Config {
    size_t transposition_table_size = 1024 * 1024 * 10;

    Config() : base_t::Config(){};

    virtual boost::program_options::options_description options(
        const std::string& category = "planner options",
        std::string prefix = "") override;
 };

  PlannerNegamax(const Config& cfg = Config());

  virtual PlannerNegamax& initialize() override;

  virtual std::string baseName() const override { return "Negamax"; }

  virtual PlannerNegamax* clone() const override { return new PlannerNegamax(*this); }
protected:
  virtual ActionVector getValidActions(
      const ConstEnvironmentPossible& origin, TEAM iTeam) const override;

  virtual EvalResult recurse_alphabeta(
      const ConstEnvironmentPossible& origin,
      size_t iDepth,
      const FitnessDepth& lowCutoff = FitnessDepth::worst(),
      const FitnessDepth& highCutoff = FitnessDepth::best(),
      size_t* nodesEvaluated=NULL) const override;

  virtual bool testAgentSelection(
      EvalResult& bestOfWorst,
      const EvalResult& worst,
      const FitnessDepth& lowCutoff,
      const ConstEnvironmentPossible& origin) const override;
  virtual bool testOtherSelection(
      EvalResult& worst,
      const EvalResult& current,
      const FitnessDepth& highCutoff,
      const ConstEnvironmentPossible& origin) const override;

  Config cfg_;

  // TODO(@drendleman) neither of these are thread safe!
  mutable TranspositionTable transpositionTable_;

  mutable OrderHeuristic orderHeuristic_;
};

#endif /* PLANNER_NEGAMAX_H */

