#include <fmt/ranges.h>
#include <gtest/gtest.h>
#include <spdlog/common.h>

#include "mock_engine_test.hpp"
#include "pokemonai/engine.h"
#include "pokemonai/pkCU.h"


// 2v2 Validation Tests
class IsValidAction2v2Test : public MockEngineTest {
 protected:
  void SetUp() override {
#if USE_LEGACY_ENGINE
    GTEST_SKIP() << "Neo-Engine test";
#endif

    MockEngineTest::SetUp();
    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .addMove(pokedex_->move("move_any_adjacent"))       // 0
            .addMove(pokedex_->move("move_all_adjacent_enemy")) // 1
            .addMove(pokedex_->move("move_all_active"))         // 2
            .addMove(pokedex_->move("move_self")))              // 3
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon2"))
            .addMove(pokedex_->move("move_any_adjacent"))       // 0
            .addMove(pokedex_->move("move_all_adjacent_enemy")) // 1
            .addMove(pokedex_->move("move_all_active")))        // 2
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon3")))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon4")));
    // clang-format on

    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setNumActivePokemon(2);
    engine_->setEnvironment(environment);
  }
};

TEST_F(IsValidAction2v2Test, TargetsNothing) {
  EXPECT_TRUE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::move(0)));
}

TEST_F(IsValidAction2v2Test, TargetsLeftEnemy) {
  EXPECT_TRUE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveEnemy(0, 0)));
}

TEST_F(IsValidAction2v2Test, TargetsRightEnemy) {
  EXPECT_TRUE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveEnemy(0, 1)));
}

TEST_F(IsValidAction2v2Test, TargetsInactiveEnemy) {
  EXPECT_FALSE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveEnemy(0, 2)));
}

TEST_F(IsValidAction2v2Test, TargetsSelf) {
  EXPECT_FALSE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveAlly(0, 0)));
}

TEST_F(IsValidAction2v2Test, TargetsAdjacentAlly) {
  EXPECT_TRUE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveAlly(0, 1)));
}

TEST_F(IsValidAction2v2Test, TargetsInactiveAlly) {
  EXPECT_FALSE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveAlly(0, 2)));
}

TEST_F(IsValidAction2v2Test, AdjacentTargeted) {
  EXPECT_FALSE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveEnemy(1, 0)));
}

TEST_F(IsValidAction2v2Test, AdjacentEnemyOnly) {
  EXPECT_TRUE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveAdjacent(1)));
}

TEST_F(IsValidAction2v2Test, AllActiveTargeted) {
  EXPECT_FALSE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 1), Action::moveEnemy(2, 0)));
}

TEST_F(IsValidAction2v2Test, AllActive) {
  EXPECT_TRUE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 1), Action::moveActive(2)));
}

TEST_F(IsValidAction2v2Test, Activates2PokemonPerTeam) {
  auto actors = engine_->initialState().getActiveActors();
  EXPECT_EQ(actors.size(), 4);
}

TEST_F(IsValidAction2v2Test, AllMoveActions) {
  auto actions = engine_->getValidMoveActions(engine_->initialState(), {TEAM_A, 0});
  // move_any_adjacent: 2 enemies + 1 ally = 3
  // move_all_adjacent_enemy: 1
  // move_all_active: 1
  // move_self: 1
  fmt::print("{}\n", fmt::streamed(actions));
  EXPECT_EQ(actions.size(), 6);
}

TEST_F(IsValidAction2v2Test, AllSwapActions) {
  auto actions = engine_->getValidSwapActions(engine_->initialState(), {TEAM_A, 0});
  fmt::print("{}\n", fmt::streamed(actions));
  EXPECT_EQ(actions.size(), 2); // swap(2), swap(3)
}

TEST_F(IsValidAction2v2Test, AllValidActionMaps) {
  auto maps = engine_->getAllValidActions(engine_->initialState(), TEAM_A);
  fmt::print("{}\n", fmt::streamed(maps));
  for (const auto& map : maps) {
    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.count(Actor(TEAM_A, 0)));
    EXPECT_TRUE(map.count(Actor(TEAM_A, 1)));
  }
  
  // Pkmn 0: 6 moves + 2 swaps = 8 actions
  // Pkmn 1: 5 moves + 2 swaps = 7 actions
  EXPECT_EQ(maps.size(), 8 * 7); // 56
}
