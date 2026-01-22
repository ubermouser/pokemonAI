#include <memory>
#include <type_traits>
#include <vector>

#include "engine_test.hpp"
#include "pokemonai/evaluator_network128.h"
#include "pokemonai/evaluator_network16.h"
#include "pokemonai/evaluator_network32.h"
#include "pokemonai/evaluator_network64.h"
#include "pokemonai/evaluator_network_large.h"
#include "pokemonai/evaluators.h"
#include "pokemonai/feature_vector.h"
#include "pokemonai/neuralNet.h"


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

  std::shared_ptr<EvaluatorNetwork> createEvaluator(const std::string& type) {
    auto cfg = evaluators::config(type);
    auto eval = std::dynamic_pointer_cast<EvaluatorNetwork>(
        evaluators::choose(type, *cfg));

    eval->setEngine(engine_);
    eval->setEnvironment(environment_);
    eval->getNetwork()->randomizeWeights();
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
  auto eval = createEvaluator("network16");
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}


TEST_F(NetworkEvaluatorTest, Network32Initialization) {
  auto eval = createEvaluator("network32");
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}


TEST_F(NetworkEvaluatorTest, Network64Initialization) {
  auto eval = createEvaluator("network64");
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}


TEST_F(NetworkEvaluatorTest, Network128Initialization) {
  auto eval = createEvaluator("network128");
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}

TEST_F(NetworkEvaluatorTest, NetworkLargeInitialization) {
  auto eval = createEvaluator("networkLarge");
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}


TEST_F(NetworkEvaluatorTest, NetworkSeedValidation) {
  auto eval = createEvaluator("networkLarge");
  eval->initialize();

  std::vector<float> seed(eval->inputSize());
  eval->seed(seed.begin(), engine_->initialState(), TEAM_A);

  std::cout << "--- Seed Vector Debug (" << seed.size() << ") ---" << std::endl;
  size_t nonZeroCount = 0;
  for (size_t i = 0; i < seed.size(); ++i) {
    EXPECT_TRUE(std::isfinite(seed[i])) << "Non-finite value at index " << i;
    EXPECT_FALSE(std::isnan(seed[i])) << "NaN value at index " << i;
    if (seed[i] != 0.0f) {
      nonZeroCount++;
      // Print first 50 non-zero values for brevity
      if (nonZeroCount < 50) {
        std::cout << "  [" << i << "] = " << seed[i] << std::endl;
      }
    }
  }
  std::cout << "--- Total non-zero values: " << nonZeroCount << " ---"
            << std::endl;
}


TEST_F(NetworkEvaluatorTest, TerminalStates) {
  auto eval = createEvaluator("network16");

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
