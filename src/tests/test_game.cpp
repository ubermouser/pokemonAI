#include <cmath>
#include <stdexcept>

#include "gen4/engine_test.hpp"
#include "pokemonai/game.h"
#include "pokemonai/planner_max.h"
#include "pokemonai/planner_random.h"

class GameTest : public Gen4EngineTest {
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
    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);

    spdlog::set_level(spdlog::level::debug);
  }

  void validateContribution(const HeatResult& result) {
    for (const auto& team : result.teams) {
      for (const auto& pk : team.pokemon) {
        EXPECT_TRUE(std::isfinite(pk.simpleContribution));
        EXPECT_TRUE(std::isfinite(pk.aggregateContribution));
      }
    }
  }
};

TEST_F(GameTest, RolloutPokemon) {
  auto game =
      Game().setMaxMatches(3).setVerbosity(3).setEnvironment(environment_nv);

  auto result = game.run();

  EXPECT_GE(result.matchesPlayed, 2);
  validateContribution(result);
}


TEST_F(GameTest, Multithreaded) {
  Game::Config cfg;
  cfg.numThreads = 2;
  cfg.maxMatches = 200;
  cfg.verbosity = 1;
  auto game = Game(cfg).setEnvironment(environment_nv);
  auto result = game.run();

  EXPECT_GE(result.matchesPlayed, 101);
  validateContribution(result);
}


TEST_F(GameTest, CustomPlanners) {
  auto cu = PkCU();
  auto game = Game()
                  .setMaxMatches(100)
                  .setEnvironment(environment_nv)
                  .setVerbosity(1)
                  .setPlanner(0, PlannerRandom().setEngine(cu))
                  .setPlanner(1, PlannerRandom().setEngine(cu));

  auto result = game.run();

  EXPECT_GE(result.matchesPlayed, 51);
  validateContribution(result);
}


TEST_F(GameTest, UninitializedCustom) {
  auto game = Game()
                  .setEnvironment(environment_nv)
                  .setPlanner(0, PlannerMax())
                  .setPlanner(1, PlannerMax());

  EXPECT_THROW({
    // uninitialized
    game.run();
  }, std::runtime_error);
}
