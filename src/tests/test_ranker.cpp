#include <gtest/gtest.h>
#include "pokemonai/ranker.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/planners.h"
#include "pokemonai/evaluators.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/game.h"
#include "pokemonai/evaluator_simple.h"

class RankerTest : public ::testing::Test {
protected:
  std::shared_ptr<PokedexStatic> pokedex_;
  std::shared_ptr<Ranker> ranker_;
  Ranker::Config ranker_cfg_;
  Game::Config game_cfg_;
  PkCU::Config engine_cfg_;

  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    ranker_cfg_.verbosity = 0;
    ranker_cfg_.minGamesPerBattlegroup = 2;
    ranker_ = std::make_shared<Ranker>(ranker_cfg_);
    ranker_->setEngine(PkCU{engine_cfg_});
    ranker_->setGame(Game{game_cfg_});
    ranker_->setStateEvaluator(EvaluatorSimple{});

    ranker_->addPlanner(planners::choose("random", *planners::config("random")));
    ranker_->addPlanner(planners::choose("max", *planners::config("max")));

    ranker_->addEvaluator(evaluators::choose("simple", *evaluators::config("simple")));
    ranker_->addEvaluator(evaluators::choose("random", *evaluators::config("random")));

    ranker_->addTeam(TeamNonVolatile::load("teams/soloTeamA.txt"));
    ranker_->addTeam(TeamNonVolatile::load("teams/soloTeamB.txt"));
  }
};

TEST_F(RankerTest, RankerRuns) {
  ranker_->initialize();
  LeagueHeat league = ranker_->rank();
  EXPECT_GT(league.games.size(), 0);
}
