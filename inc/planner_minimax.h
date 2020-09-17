/* 
 * File:   planner_minimax.h
 * Author: drendleman
 *
 * Created on September 16, 2020, 1:57 PM
 */

#ifndef PLANNER_MINIMAX_H
#define PLANNER_MINIMAX_H

#include <boost/program_options.hpp>
#include <string>

#include "order_heuristic.h"
#include "planner_maximin.h"
#include "transposition_table.h"

class PlannerMiniMax : public PlannerMaxiMin {
public:
  using base_t = PlannerMaxiMin;

  struct Config: public base_t::Config {
    size_t transposition_table_size = 10000000;

    Config() : base_t::Config() {};

    virtual boost::program_options::options_description options(
        const std::string& category="planner options", std::string prefix="") override;
  };

  PlannerMiniMax(const Config& cfg = Config());

  virtual PlannerMiniMax& initialize() override;

  virtual std::string baseName() const override { return "MiniMaxPlanner"; }

  virtual PlannerMiniMax* clone() const override { return new PlannerMiniMax(*this); }
protected:
  virtual ActionVector getValidActions(
      const ConstEnvironmentPossible& origin, size_t iTeam) const override;

  virtual EvalResult recurse_alphabeta(
      const ConstEnvironmentPossible& origin,
      size_t iDepth,
      const Fitness& lowCutoff = Fitness::worst(),
      const Fitness& highCutoff = Fitness::best(),
      size_t* nodesEvaluated=NULL) const override;

  virtual bool testAgentCutoff(
      EvalResult& bestOfWorst, 
      const EvalResult& worst,
      const ConstEnvironmentPossible& origin) const override;
  virtual bool testOtherCutoff(
      EvalResult& worst, 
      const EvalResult& current,
      const ConstEnvironmentPossible& origin) const override;

  Config cfg_;

  mutable TranspositionTable transpositionTable_;

  mutable OrderHeuristic orderHeuristic_;
};

#endif /* PLANNER_MINIMAX_H */

