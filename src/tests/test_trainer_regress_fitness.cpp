#include <gtest/gtest.h>

#include "engine_test.hpp"
#include "pokemonai/evaluator_network16.h"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/evaluators.h"
#include "pokemonai/game.h"
#include "pokemonai/planners.h"
#include "pokemonai/ranker.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/trainable_neural_net.h"
#include "pokemonai/trainer_regress_fitness.h"

class TrainerRegressFitnessTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Setup Ranker
    Ranker::Config ranker_cfg;
    ranker_cfg.verbosity = 0;
    ranker_cfg.minGamesPerBattlegroup = 5;
    ranker_cfg.numThreads = 1;
    ranker_ = std::make_shared<Ranker>(ranker_cfg);
    ranker_->setEngine(*engine_);
    ranker_->setGame(Game{});
    ranker_->setStateEvaluator(EvaluatorSimple{});

    ranker_->addPlanner(
        planners::choose("random", *planners::config("random")));
    ranker_->addEvaluator(
        evaluators::choose("simple", *evaluators::config("simple")));

    ranker_->addTeam(TeamNonVolatile::load("teams/gen4/dualTeamA.txt"));
    ranker_->addTeam(TeamNonVolatile::load("teams/gen4/dualTeamB.txt"));

    ranker_->initialize();

    spdlog::set_level(spdlog::level::info);
  }

  std::shared_ptr<Ranker> ranker_;
};


TEST_F(TrainerRegressFitnessTest, FitOnLeagueHeatReducesLoss) {
  // 1. Generate LeagueHeat using the fixture's ranker
  LeagueHeat lHeat = ranker_->rank();

  // 2. Setup Trainer
  auto fv = std::make_shared<evaluator_network16>();

  TrainableNeuralNet::Config netCfg;
  netCfg.learningRate = 0.001;
  auto net = std::make_shared<TrainableNeuralNet>(netCfg, *fv);
  net->initialize();

  TrainerRegressFitness::Config tCfg;
  tCfg.logInterval = 1;
  tCfg.batchSize = 8;
  tCfg.seed = 42;
  tCfg.numEpochs = 5;
  tCfg.discountFactor = 0.9;
  tCfg.trainOnOwnData = false;  // Test users simple/random planners

  TrainerRegressFitness trainer(fv, net, tCfg);

  float initialLoss = trainer.predict(lHeat);
  trainer.fit(lHeat);
  float finalLoss = trainer.predict(lHeat);

  EXPECT_LT(finalLoss, initialLoss);
}
