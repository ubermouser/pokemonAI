#include <gtest/gtest.h>
#include "pokemonai/teambuilder.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/planners.h"
#include "pokemonai/evaluators.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/game.h"
#include "pokemonai/evaluator_simple.h"

class TeamBuilderTest : public ::testing::Test {
protected:
  std::shared_ptr<PokedexStatic> pokedex_;
  std::shared_ptr<TeamBuilder> teambuilder_;
  TeamBuilder::Config teambuilder_cfg_;
  Game::Config game_cfg_;
  PkCU::Config engine_cfg_;

  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    teambuilder_cfg_.verbosity = 10;
    teambuilder_cfg_.minGamesPerBattlegroup = 1;
    teambuilder_cfg_.maxGenerations = 1;
    teambuilder_cfg_.teamPopulationSize = {10, 0, 0, 0, 0, 0};
    teambuilder_ = std::make_shared<TeamBuilder>(teambuilder_cfg_);
    teambuilder_->setEngine(PkCU{engine_cfg_});
    teambuilder_->setGame(Game{game_cfg_});
    teambuilder_->setStateEvaluator(EvaluatorSimple{});

    teambuilder_->addPlanner(planners::choose("random", *planners::config("random")));
    teambuilder_->addEvaluator(evaluators::choose("simple", *evaluators::config("simple")));
  }
};

TEST_F(TeamBuilderTest, TeamBuilderRuns) {
  teambuilder_->initialize();
  LeagueHeat league = teambuilder_->evolve();
  EXPECT_GT(league.games.size(), 0);
}
