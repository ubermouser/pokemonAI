#include <gtest/gtest.h>

#include "gen4/engine_test.hpp"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/evaluators.h"
#include "pokemonai/game.h"
#include "pokemonai/planners.h"
#include "pokemonai/ranker.h"
#include "pokemonai/team_nonvolatile.h"

class RankerTest : public Gen4EngineTest {
 protected:
  std::shared_ptr<Ranker> ranker_;
  Ranker::Config ranker_cfg_;
  Game::Config game_cfg_;

  void SetUp() override {
    Gen4EngineTest::SetUp();

    ranker_cfg_.verbosity = 3;
    ranker_cfg_.minGamesPerBattlegroup = 2;
    ranker_cfg_.numThreads = 2;
    ranker_ = std::make_shared<Ranker>(ranker_cfg_);
    ranker_->setEngine(*engine_);
    ranker_->setGame(Game{game_cfg_});
    ranker_->setStateEvaluator(EvaluatorSimple{});

    ranker_->addPlanner(planners::choose("random", *planners::config("random")));
    ranker_->addPlanner(planners::choose("max", *planners::config("max")));

    ranker_->addEvaluator(evaluators::choose("simple", *evaluators::config("simple")));
    ranker_->addEvaluator(evaluators::choose("random", *evaluators::config("random")));

    ranker_->addTeam(TeamNonVolatile::load("teams/gen4/dualTeamA.txt"));
    ranker_->addTeam(TeamNonVolatile::load("teams/gen4/dualTeamB.txt"));

    spdlog::set_level(spdlog::level::debug);
  }
};

TEST_F(RankerTest, RankerRuns) {
  ranker_->initialize();
  LeagueHeat league = ranker_->rank();
  EXPECT_GT(league.games.size(), 0);
}

TEST_F(RankerTest, CalculateLeagueStats) {
  ranker_->initialize();
  LeagueHeat league = ranker_->rank();
  league.calculateCounts();

  // We added some teams, each with some pokemon.
  // We should have at least some counts in our stats.
  EXPECT_FALSE(league.counts.pokemon.empty());
  EXPECT_FALSE(league.counts.abilities.empty());
  EXPECT_FALSE(league.counts.natures.empty());
  EXPECT_FALSE(league.counts.types.empty());
  EXPECT_FALSE(league.counts.moves.empty());
}
