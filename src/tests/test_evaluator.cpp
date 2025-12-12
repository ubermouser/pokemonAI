#include <gtest/gtest.h>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"

#include "pokemonai/evaluator.h"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/evaluator_random.h"
#include "pokemonai/evaluator_montecarlo.h"


class EvaluatorTest : public ::testing::Test {
protected:
  void SetUp() override {
    verbose = 4;
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("cut"))
          .addMove(pokedex_->move("swords dance"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .addMove(pokedex_->move("cut"))
          .addMove(pokedex_->move("charm"))
          .setLevel(100));
    environment_ = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(environment_);
  }

  std::shared_ptr<EnvironmentNonvolatile> environment_;
  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
};


void validateTerminalState(Evaluator& eval, const ConstEnvironmentVolatile& envp, fpType fitness=0.0) {
  EvalResult losingFitness = eval.evaluate(envp, TEAM_A);
  EvalResult winningFitness = eval.evaluate(envp, TEAM_B);

  EXPECT_EQ(losingFitness.fitness, Fitness{fitness});
  EXPECT_EQ(winningFitness.fitness, Fitness{1.0 - fitness});
  EXPECT_TRUE(losingFitness.fullyEvaluated());
  EXPECT_TRUE(winningFitness.fullyEvaluated());
}


void validateNonTerminalState(Evaluator& eval, const ConstEnvironmentVolatile& envp) {
  EvalResult agentFitness = eval.evaluate(envp, TEAM_A);
  EvalResult otherFitness = eval.evaluate(envp, TEAM_B);

  EXPECT_LT(agentFitness, Fitness::best());
  EXPECT_GT(otherFitness, Fitness::worst());
  EXPECT_TRUE(agentFitness.depth == 0);
  EXPECT_TRUE(otherFitness.depth == 0);
}


std::vector<std::shared_ptr<Evaluator>> buildEvaluators(
    const std::shared_ptr<EnvironmentNonvolatile>& nv, const std::shared_ptr<PkCU>& engine) {
  std::vector<std::shared_ptr<Evaluator>> evaluators;
  evaluators.push_back(std::make_shared<EvaluatorSimple>());
  evaluators.push_back(std::make_shared<EvaluatorRandom>());
  evaluators.push_back(std::make_shared<EvaluatorMonteCarlo>());

  for (auto& evaluator : evaluators) {
    evaluator->setEngine(engine);
    evaluator->setEnvironment(nv);
    EXPECT_NO_THROW(evaluator->initialize());
  }

  return evaluators;
}


TEST_F(EvaluatorTest, TerminalStatesProduceTerminalFitness) {
  std::vector<std::shared_ptr<Evaluator>> evaluators = buildEvaluators(environment_, engine_);

  auto terminalStateData = engine_->initialState().data();
  auto terminalTieStateData = terminalStateData;
  auto terminalState = EnvironmentVolatile{*environment_, terminalStateData};
  auto terminalTieState = EnvironmentVolatile{*environment_, terminalTieStateData};

  terminalState.getTeam(0).cSetHP(0); // kill first pokemon
  terminalTieState.getTeam(0).cSetHP(0); // kill both first pokemon
  terminalTieState.getTeam(1).cSetHP(0); 

  for (auto& evaluator : evaluators) {
    validateTerminalState(*evaluator, terminalState, 0.0);
    validateTerminalState(*evaluator, terminalTieState, 0.5);
  }
}


TEST_F(EvaluatorTest, NonTerminalStateFitness) {
  std::vector<std::shared_ptr<Evaluator>> evaluators = buildEvaluators(environment_, engine_);

  for (auto& evaluator : evaluators) {
    validateNonTerminalState(*evaluator, engine_->initialState());
  }
}
