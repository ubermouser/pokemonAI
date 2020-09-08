/* 
 * File:   evaluator_montecarlo.h
 * Author: drendleman
 *
 * Created on August 20, 2020, 5:43 PM
 */

#ifndef EVALUATOR_MONTECARLO_H
#define EVALUATOR_MONTECARLO_H

#include <memory>

#include "game.h"
#include "evaluator.h"

class EvaluatorMonteCarlo : public Evaluator {
public:
  struct Config : public Evaluator::Config {
    size_t maxRollouts = 250;

    size_t maxPlies = 125;

    size_t numThreads = 0;

    Config() : Evaluator::Config() {};
  };

  EvaluatorMonteCarlo(const Config& cfg = Config());
  virtual ~EvaluatorMonteCarlo() override {};

  virtual EvaluatorMonteCarlo* clone() const override { return new EvaluatorMonteCarlo(*this); }

  virtual EvaluatorMonteCarlo& setEnvironment(
      const std::shared_ptr<const EnvironmentNonvolatile>& env) override;

  virtual EvaluatorMonteCarlo& initialize() override;

  virtual EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;
protected:
  Config cfg_;

  mutable std::shared_ptr<Game> game_;
};

#endif /* EVALUATOR_MONTECARLO_H */

