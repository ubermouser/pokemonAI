#include <gtest/gtest.h>
#include "pokemonai/trainer.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/planners.h"
#include "pokemonai/evaluators.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/game.h"
#include "pokemonai/evaluator_simple.h"

class TrainerTest : public ::testing::Test {
protected:
  std::shared_ptr<PokedexStatic> pokedex_;
  std::shared_ptr<Trainer> trainer_;
  Trainer::Config trainer_cfg_;
  Game::Config game_cfg_;
  PkCU::Config engine_cfg_;

  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    trainer_cfg_.verbosity = 10;
    trainer_cfg_.minGamesPerBattlegroup = 1;
    trainer_cfg_.maxGenerations = 1;
    trainer_cfg_.teamPopulationSize = {10, 0, 0, 0, 0, 0};
    trainer_ = std::make_shared<Trainer>(trainer_cfg_);
    trainer_->setEngine(PkCU{engine_cfg_});
    trainer_->setGame(Game{game_cfg_});
    trainer_->setStateEvaluator(EvaluatorSimple{});

    trainer_->addPlanner(planners::choose("random", *planners::config("random")));
    trainer_->addEvaluator(evaluators::choose("simple", *evaluators::config("simple")));
  }
};

TEST_F(TrainerTest, TrainerRuns) {
  trainer_->initialize();
  LeagueHeat league = trainer_->evolve();
  EXPECT_GT(league.games.size(), 0);
}
