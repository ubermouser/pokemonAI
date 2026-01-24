#include <fmt/format.h>

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
    if (auto tEval =
            std::dynamic_pointer_cast<TrainableEvaluatorNetwork>(eval)) {
      tEval->getTrainableNetwork()->randomizeWeights();
    }
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


class NetworkEvaluatorParamTest
    : public NetworkEvaluatorTest,
      public ::testing::WithParamInterface<std::string> {};

INSTANTIATE_TEST_SUITE_P(
    AllNetworks,
    NetworkEvaluatorParamTest,
    ::testing::Values(
        "tnetwork16",
        "tnetwork32",
        "tnetwork64",
        "tnetwork128",
        "tnetworkLarge"));


TEST_P(NetworkEvaluatorParamTest, Initialization) {
  auto eval = createEvaluator(GetParam());
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}


TEST_P(NetworkEvaluatorParamTest, SeedValidation) {
  auto eval = createEvaluator(GetParam());
  eval->initialize();

  std::vector<float> seed(eval->inputSize());
  eval->seed(seed.begin(), engine_->initialState(), TEAM_A);

  fmt::print(
      "--- Seed Vector Debug ({}, size: {}) ---\n", GetParam(), seed.size());
  size_t nonZeroCount = 0;
  for (size_t i = 0; i < seed.size(); ++i) {
    EXPECT_TRUE(std::isfinite(seed[i]))
        << "Non-finite value at index " << i << " for " << GetParam();
    EXPECT_FALSE(std::isnan(seed[i]))
        << "NaN value at index " << i << " for " << GetParam();
    if (seed[i] != 0.0f) {
      nonZeroCount++;
      // Print first 50 non-zero values for brevity
      if (nonZeroCount < 50) { fmt::print("  [{}] = {}\n", i, seed[i]); }
    }
  }
  fmt::print("--- Total non-zero values: {} ---\n", nonZeroCount);
}


TEST_P(NetworkEvaluatorParamTest, FrozenLoadFailure) {
  std::string fType = GetParam().substr(1);  // remove 't'
  auto cfg = std::static_pointer_cast<EvaluatorNetwork::Config>(
      evaluators::config(fType));
  cfg->netConfig.modelPath = "";  // Ensure empty path
  auto eval = evaluators::choose(fType, *cfg);

  eval->setEngine(engine_);
  eval->setEnvironment(environment_);

  // Frozen network initialization should throw because modelPath is empty
  EXPECT_THROW(eval->initialize(), std::invalid_argument);
}


TEST_P(NetworkEvaluatorParamTest, TrainableLoadSuccess) {
  std::string tType = GetParam();
  auto cfg = std::static_pointer_cast<TrainableEvaluatorNetwork::Config>(
      evaluators::config(tType));
  cfg->netConfig.modelPath = "";  // Ensure empty path
  auto eval = evaluators::choose(tType, *cfg);

  eval->setEngine(engine_);
  eval->setEnvironment(environment_);

  // Trainable network initialization should succeed even with empty modelPath
  EXPECT_NO_THROW(eval->initialize());
  validateNetworkNonTerminalState(*eval, engine_->initialState());
}


TEST_P(NetworkEvaluatorParamTest, SaveLoadParity) {
  std::string tType = GetParam();
  std::string fType = GetParam().substr(1);  // remove 't'
  auto tCfg = std::static_pointer_cast<TrainableEvaluatorNetwork::Config>(
      evaluators::config(tType));
  tCfg->netConfig.modelPath = "";
  auto tEval = std::dynamic_pointer_cast<TrainableEvaluatorNetwork>(
      evaluators::choose(tType, *tCfg));

  tEval->setEngine(engine_);
  tEval->setEnvironment(environment_);
  tEval->initialize();
  tEval->getTrainableNetwork()->randomizeWeights();

  // Save the trainable network
  std::string tempModel = "temp_model.pt";
  {
    std::ofstream oFile(tempModel, std::ios::binary);
    tEval->getTrainableNetwork()->output(oFile);
  }

  // Load it as a frozen network
  auto fCfg = std::static_pointer_cast<EvaluatorNetwork::Config>(
      evaluators::config(fType));
  fCfg->netConfig.modelPath = tempModel;
  auto fEval = std::dynamic_pointer_cast<EvaluatorNetwork>(
      evaluators::choose(fType, *fCfg));

  fEval->setEngine(engine_);
  fEval->setEnvironment(environment_);
  fEval->initialize();

  // Verify outputs are identical
  auto state = engine_->initialState();
  EvalResult tRes = tEval->evaluate(state, TEAM_A);
  EvalResult fRes = fEval->evaluate(state, TEAM_A);

  EXPECT_NEAR(tRes.fitness.value(), fRes.fitness.value(), 1e-6);

  // Cleanup
  std::remove(tempModel.c_str());
}


TEST_P(NetworkEvaluatorParamTest, CloneInitialization) {
  std::string tType = GetParam();
  std::string fType = GetParam().substr(1);  // remove 't'

  auto tCfg = std::static_pointer_cast<TrainableEvaluatorNetwork::Config>(
      evaluators::config(tType));
  tCfg->netConfig.modelPath = "";
  auto tEval = std::dynamic_pointer_cast<TrainableEvaluatorNetwork>(
      evaluators::choose(tType, *tCfg));

  tEval->setEngine(engine_);
  tEval->setEnvironment(environment_);
  tEval->initialize();  // Initializes the network (random weights)

  // Clone the evaluator (which clones the network)
  std::unique_ptr<EvaluatorNetwork> fEval(
      dynamic_cast<EvaluatorNetwork*>(tEval->clone()));

  // fEval is a frozen network type, and its internal config likely has no path.
  // However, it already has an initialized network.
  // initialize() should succeed.
  EXPECT_NO_THROW(fEval->initialize());

  // Verify output parity
  auto state = engine_->initialState();
  EvalResult tRes = tEval->evaluate(state, TEAM_A);
  EvalResult fRes = fEval->evaluate(state, TEAM_A);
  EXPECT_NEAR(tRes.fitness.value(), fRes.fitness.value(), 1e-6);
}
