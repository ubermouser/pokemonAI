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
  struct Config {
    size_t maxRollouts = 250;

    size_t maxPlies = 125;

    Config(){}
  };

  EvaluatorMonteCarlo(const Config& cfg = Config());
  virtual ~EvaluatorMonteCarlo() override {};

  virtual EvaluatorMonteCarlo* clone() const override { return new EvaluatorMonteCarlo(*this); }

  virtual EvaluatorMonteCarlo& setEnvironment(
      const std::shared_ptr<const EnvironmentNonvolatile>& env) override;

  virtual bool isInitialized() const override;

  virtual EvalResult_t calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;
protected:
  Config cfg_;

  mutable std::shared_ptr<Game> game_;
};

#endif /* EVALUATOR_MONTECARLO_H */

