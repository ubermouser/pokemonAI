#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

#include "engine_test.hpp"
#include "pokemonai/evaluator_network16.h"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/evaluators.h"
#include "pokemonai/planners.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/trainer.h"

class TrainerTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    trainer_cfg_.verbosity = 2;
    trainer_cfg_.minGamesPerBattlegroup = 1;
    trainer_cfg_.maxGenerations = 1;
    trainer_cfg_.saveOnCompletion = true;
    trainer_cfg_.teamPopulationSize = {4, 0, 0, 0, 0, 0};
    trainer_cfg_.training.numEpochs = 1;
    trainer_cfg_.training.batchSize = 2;

    trainer_ = std::make_shared<Trainer>(trainer_cfg_);
    trainer_->setEngine(*engine_);

    Game::Config game_cfg;
    game_cfg.storeSubcomponents = true;
    trainer_->setGame(Game{game_cfg});

    trainer_->setStateEvaluator(EvaluatorSimple{});

    trainer_->addPlanner(
        planners::choose("random", *planners::config("random")));

    auto eval_cfg = std::static_pointer_cast<EvaluatorNetwork::Config>(
        evaluators::config("network16"));
    eval_cfg->netConfig.modelPath = "test_trainer_model.pt";
    eval_cfg->netConfig.architecture = {8};
    eval_cfg->netConfig.randomWeights = true;

    trainer_->addEvaluator(evaluators::choose("network16", *eval_cfg));
    trainer_->addEvaluator(
        evaluators::choose("simple", *evaluators::config("simple")));

    trainer_->addTeam(TeamNonVolatile::load("teams/gen4/dualTeamA.txt"));
    trainer_->addTeam(TeamNonVolatile::load("teams/gen4/dualTeamB.txt"));

    spdlog::set_level(spdlog::level::info);
  }

  Trainer::Config trainer_cfg_;
  std::shared_ptr<Trainer> trainer_;
};

TEST_F(TrainerTest, TrainerEvolvesAndTrains) {
  trainer_->initialize();

  if (boost::filesystem::exists("test_trainer_model.pt")) {
    boost::filesystem::remove("test_trainer_model.pt");
  }

  LeagueHeat league = trainer_->evolve();

  EXPECT_GT(league.games.size(), 0);

  // Check if model was saved
  EXPECT_TRUE(boost::filesystem::exists("test_trainer_model.pt"));
  boost::filesystem::remove("test_trainer_model.pt");
}
