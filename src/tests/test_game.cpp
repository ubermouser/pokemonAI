#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include <inc/engine.h>
#include <inc/game.h>
#include <inc/pokedex_static.h>
#include <inc/pkCU.h>
#include <inc/planner_random.h>
#include <inc/planner_max.h>


class GameTest : public ::testing::Test {
protected:
  void SetUp() override {
    verbose = 4;
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("charmander"))
          .addMove(pokedex_->getMoves().at("cut"))
          .addMove(pokedex_->getMoves().at("swords dance"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("bulbasaur"))
          .addMove(pokedex_->getMoves().at("cut"))
          .addMove(pokedex_->getMoves().at("charm"))
          .setLevel(100));
    environment_ = EnvironmentNonvolatile(team_a, team_b, true);
  }

  EnvironmentNonvolatile environment_;
  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
};


TEST_F(GameTest, RolloutPokemon) {
  auto game = Game()
      .setMaxMatches(3)
      .setVerbosity(3)
      .setEnvironment(environment_);

  auto result = game.rollout();

  EXPECT_GE(result.matchesPlayed, 2);
}


TEST_F(GameTest, Multithreaded) {
  Game::Config cfg;
  cfg.numThreads = 2;
  cfg.maxMatches = 200;
  cfg.verbosity = 1;
  auto game = Game(cfg)
      .setEnvironment(environment_);
  auto result = game.rollout();

  EXPECT_GE(result.matchesPlayed, 101);
}


TEST_F(GameTest, CustomPlanners) {
  auto cu = PkCU();
  auto game = Game()
      .setMaxMatches(100)
      .setEnvironment(environment_)
      .setVerbosity(1)
      .setPlanner(0, PlannerRandom().setEngine(cu))
      .setPlanner(1, PlannerRandom().setEngine(cu));

  auto result = game.rollout();

  EXPECT_GE(result.matchesPlayed, 51);
}


TEST_F(GameTest, UninitializedCustomPlanners) {
  auto game = Game()
      .setEnvironment(environment_)
      .setPlanner(0, PlannerMax())
      .setPlanner(1, PlannerMax());

  EXPECT_THROW({
    // uninitialized
    game.rollout();
  }, std::runtime_error);
}
