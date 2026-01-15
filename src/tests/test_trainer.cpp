#include <gtest/gtest.h>
#include "engine_test.hpp"
#include "pokemonai/trainer.h"
#include "pokemonai/evaluator_network16.h"
#include "pokemonai/trainable_neural_net.h"
#include "pokemonai/game.h"

class TrainerTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

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
    environment_nv = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);

    spdlog::set_level(spdlog::level::info);
  }

  std::shared_ptr<EnvironmentNonvolatile> environment_nv;
};

TEST_F(TrainerTest, FitReducesLoss) {
  auto fv = std::make_shared<evaluator_network16>();
  fv->setEnvironment(environment_nv);
  
  TrainableNeuralNet::Config cfg;
  cfg.learningRate = 0.001;
  auto net = std::make_shared<TrainableNeuralNet>(cfg, *fv);

  Trainer::Config tCfg;
  tCfg.logInterval = 1;
  tCfg.batchSize = 8;
  tCfg.seed = 42;
  tCfg.numEpochs = 5;
  
  // Generate a real HeatResult
  auto game = Game().setMaxMatches(5).setVerbosity(0).setEnvironment(*environment_nv);
  HeatResult hResult = game.run();
  Trainer trainer(fv, net, environment_nv, tCfg);
  
  float initialLoss = trainer.predict(hResult);
  trainer.fit(hResult);
  float finalLoss = trainer.predict(hResult);

  ASSERT_FALSE(hResult.gameResults.empty());
  EXPECT_LT(finalLoss, initialLoss);
}
