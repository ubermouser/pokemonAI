/* 
 * File:   evaluator_montecarlo.h
 * Author: drendleman
 *
 * Created on August 20, 2020, 5:43 PM
 */

#ifndef EVALUATOR_MONTECARLO_H
#define EVALUATOR_MONTECARLO_H

#include <memory>
#include <boost/program_options.hpp>

#include "game.h"
#include "evaluator.h"

class EvaluatorMonteCarlo : public Evaluator {
public:
  struct Config : public Evaluator::Config {
    size_t maxRollouts = 600;

    size_t maxPlies = 20;

    size_t numThreads = 0;

    double moveChance = 0.75;

    Config() : Evaluator::Config() {};

    virtual boost::program_options::options_description options(
        const std::string& category="evaluator options", std::string prefix="") override;
  };

  EvaluatorMonteCarlo(const Config& cfg = Config());
  virtual ~EvaluatorMonteCarlo() override {};

  virtual EvaluatorMonteCarlo* clone() const override { return new EvaluatorMonteCarlo(*this); }

  virtual EvaluatorMonteCarlo& setEnvironment(
      const std::shared_ptr<const EnvironmentNonvolatile>& env) override;

  virtual EvaluatorMonteCarlo& initialize() override;

  virtual EvalResult calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;
protected:
  Config cfg_;

  std::shared_ptr<Game> game_;
};

#endif /* EVALUATOR_MONTECARLO_H */

