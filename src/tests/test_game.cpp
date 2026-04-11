#include <cmath>
#include <stdexcept>

#include "gen4/engine_test.hpp"
#include "pokemonai/game.h"
#include "pokemonai/pkai.h"
#include "pokemonai/planner_max.h"
#include "pokemonai/planner_random.h"

class GameTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle"))
          .addMove(pokedex_->move("tackle"))
          .addMove(pokedex_->move("tail whip"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .addMove(pokedex_->move("tackle"))
          .addMove(pokedex_->move("growl"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("cut"))
          .addMove(pokedex_->move("defense curl"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu"))
          .addMove(pokedex_->move("strength"))
          .addMove(pokedex_->move("flash"))
          .setLevel(100));
    // clang-format on
    environment_nv = EnvironmentNonvolatile(team, team, true);

    spdlog::set_level(spdlog::level::debug);
  }

  EnvironmentNonvolatile oneSidedTeam() {
    // clang-format off
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("cut"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .addMove(pokedex_->move("growl"))
          .setLevel(100));
    // clang-format on
    return EnvironmentNonvolatile(team_a, team_b, true);
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


TEST_F(GameTest, KODetection) {
  auto game =
      Game().setMaxMatches(1).setVerbosity(0).setEnvironment(oneSidedTeam());
  auto result = game.run();

  // Charmander (TEAM_A, Pokemon 0) should have exactly 1 KO against Bulbasaur
  // (TEAM_B, Pokemon 0)
  EXPECT_EQ(result.teams[TEAM_A].pokemon[0].encounters[0].numKOs, 1);
  // Bulbasaur should have 0 KOs
  EXPECT_EQ(result.teams[TEAM_B].pokemon[0].encounters[0].numKOs, 0);
}


TEST_F(GameTest, SetTeamInitialization) {
  // clang-format off
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("charmander"))
        .addMove(pokedex_->move("cut"))
        .setLevel(100));
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("bulbasaur"))
        .addMove(pokedex_->move("cut"))
        .setLevel(100));

  auto game = Game()
      .setMaxMatches(1)
      .setVerbosity(0)
      .setEngine(PkCU())
      .setTeam(0, team_a)
      .setTeam(1, team_b);
  // clang-format on

  // This should not crash
  EXPECT_NO_THROW({ game.run(); });
}


TEST_F(GameTest, Supports2v2) {
  engine_->setNumActivePokemon(2);
  auto game = Game()
                  .setMaxMatches(1)
                  .setEnvironment(environment_nv)
                  .setEngine(engine_)
                  .setVerbosity(3);
  auto result = game.run();

  EXPECT_EQ(result.matchesPlayed, 1);
  validateContribution(result);
}


TEST_F(GameTest, DISABLED_Supports3v3) {
  engine_->setNumActivePokemon(3);
  auto game = Game()
                  .setMaxMatches(1)
                  .setEnvironment(environment_nv)
                  .setEngine(engine_)
                  .setVerbosity(3);
  auto result = game.run();

  EXPECT_EQ(result.matchesPlayed, 1);
  validateContribution(result);
}
