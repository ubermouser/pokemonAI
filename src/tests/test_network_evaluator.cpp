#include "engine_test.hpp"
#include "pokemonai/evaluator_network16.h"
#include "pokemonai/evaluator_network32.h"
#include "pokemonai/evaluator_network64.h"
#include "pokemonai/evaluator_network128.h"
#include "pokemonai/neuralNet.h"
#include <vector>
#include <memory>

class NetworkEvaluatorTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    engine_->setAllowInvalidMoves(false);

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

    spdlog::set_level(spdlog::level::debug);
  }

  template<typename T>
  std::shared_ptr<T> createEvaluator() {
    std::vector<size_t> widths = { T::numInputNeurons, 16, T::numOutputNeurons };
    neuralNet net(widths.begin(), widths.end());
    net.randomizeWeights();
    
    auto eval = std::make_shared<T>(net);
    eval->setEngine(engine_);
    eval->setEnvironment(environment_);
    return eval;
  }

  std::shared_ptr<EnvironmentNonvolatile> environment_;
};

void validateNetworkTerminalState(Evaluator& eval, const ConstEnvironmentVolatile& envp, fpType fitness=0.0) {
  EvalResult agentFitness = eval.evaluate(envp, TEAM_A);
  EvalResult otherFitness = eval.evaluate(envp, TEAM_B);

  EXPECT_EQ(agentFitness.fitness.value(), fitness);
  EXPECT_EQ(otherFitness.fitness.value(), 1.0 - fitness);
  EXPECT_TRUE(agentFitness.fullyEvaluated());
  EXPECT_TRUE(otherFitness.fullyEvaluated());
}

void validateNetworkNonTerminalState(Evaluator& eval, const ConstEnvironmentVolatile& envp) {
  EvalResult agentFitness = eval.evaluate(envp, TEAM_A);
  EvalResult otherFitness = eval.evaluate(envp, TEAM_B);

  EXPECT_GE(agentFitness.fitness.value(), 0.0);
  EXPECT_LE(agentFitness.fitness.value(), 1.0);
  EXPECT_GE(otherFitness.fitness.value(), 0.0);
  EXPECT_LE(otherFitness.fitness.value(), 1.0);
}

TEST_F(NetworkEvaluatorTest, Network16Initialization) {
  auto eval = createEvaluator<evaluator_network16>();
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}

TEST_F(NetworkEvaluatorTest, Network32Initialization) {
  auto eval = createEvaluator<evaluator_network32>();
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}

TEST_F(NetworkEvaluatorTest, Network64Initialization) {
  auto eval = createEvaluator<evaluator_network64>();
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}

TEST_F(NetworkEvaluatorTest, Network128Initialization) {
  auto eval = createEvaluator<evaluator_network128>();
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}

TEST_F(NetworkEvaluatorTest, TerminalStates) {
  auto eval = createEvaluator<evaluator_network16>();
  
  auto terminalStateData = engine_->initialState().data();
  auto terminalTieStateData = terminalStateData;
  auto terminalState = EnvironmentVolatile{*environment_, terminalStateData};
  auto terminalTieState = EnvironmentVolatile{*environment_, terminalTieStateData};

  terminalState.getTeam(0).cSetHP(0); // kill first pokemon
  terminalTieState.getTeam(0).cSetHP(0); // kill both first pokemon
  terminalTieState.getTeam(1).cSetHP(0); 

  validateNetworkTerminalState(*eval, terminalState, 0.0);
  validateNetworkTerminalState(*eval, terminalTieState, 0.5);
}
