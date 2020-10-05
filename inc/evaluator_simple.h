#ifndef SIMPLE_EVALUATOR_H
#define SIMPLE_EVALUATOR_H

#include "evaluator.h"

class EvaluatorSimple: public Evaluator {
public:
  struct Config : public Evaluator::Config {
    /* proportion of a move's fitness awarded for having moves left. */
    fpType canMoveBias = 0.05;

    /* proportion of a pokemon's fitness dedicated for moves. */
    fpType movesBias = 0.1;

    /* proportion of a pokemon's fitness awarded for free for being alive. */
    fpType aliveBias = 0.05;

    /* proportion of a team's fitness awarded for free for having any single pokemon alive. */
    fpType teamAliveBias = 0.05;

    Config() : Evaluator::Config() {};
  };

  EvaluatorSimple(const Config& cfg = Config());
  virtual ~EvaluatorSimple() override {};
  EvaluatorSimple(const EvaluatorSimple& other) = default;

  EvaluatorSimple* clone() const override { return new EvaluatorSimple(*this); }

  EvaluatorSimple& initialize() override;
protected:
  EvalResult calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;

  fpType fitness_move(const ConstMoveVolatile& mV) const;
  fpType fitness_pokemon(const ConstPokemonVolatile& pV) const;
  fpType fitness_team(const ConstTeamVolatile& tV) const;
  
  virtual std::string baseName() const override { return "Simple"; }
  virtual void resetName() override;

  Config cfg_;
};

#endif /* SIMPLE_EVALUATOR_H */
