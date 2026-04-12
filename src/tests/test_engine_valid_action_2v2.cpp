#include <fmt/ranges.h>
#include <gtest/gtest.h>
#include <spdlog/common.h>

#include "mock_engine_test.hpp"
#include "pokemonai/engine.h"
#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/environment_possible.h"
#include "pokemonai/pkCU.h"


// 2v2 Validation Tests
class IsValidAction2v2Test : public MockEngineTest {
 protected:
  void SetUp() override {

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
            .addMove(pokedex_->move("move_all_active"))         // 2
            .addMove(pokedex_->move("move_suicide")))           // 3
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon3")))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon4")));
    // clang-format on

    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setNumActivePokemon(2);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setup_faintedTA1() {
    // clang-format off
    // TEAM_A 1 (test_pokemon2) uses move_suicide (move index 3)
    return engine_->updateState(
        engine_->initialState(),
        {{Actor(TEAM_A, 0), Action::wait()},
        {Actor(TEAM_A, 1), Action::move(3)},
        {Actor(TEAM_B, 0), Action::wait()},
        {Actor(TEAM_B, 1), Action::wait()}}
    );
    // clang-format on
  }

  ConstEnvironmentVolatile setup_2v2() {
    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(environment_nv.teammate(TEAM_A, 0))
        .addPokemon(environment_nv.teammate(TEAM_A, 1));
    // clang-format on

    auto environment_2v2 = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_2v2);

    return engine_->initialState();
  }

  PossibleEnvironments setup_2v2_faintedTA1() {
    auto state_2v2 = setup_2v2();

    // clang-format off
    // TEAM_A 1 (test_pokemon2) uses move_suicide (move index 3)
    return engine_->updateState(
        state_2v2,
        {{Actor(TEAM_A, 0), Action::wait()},
        {Actor(TEAM_A, 1), Action::move(3)},
        {Actor(TEAM_B, 0), Action::wait()},
        {Actor(TEAM_B, 1), Action::wait()}}
    );
    // clang-format on
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


TEST_F(IsValidAction2v2Test, FaintedPokemonNotConsideredActive) {
  auto state = setup_faintedTA1();
  auto actors = state.where1().getEnv().getActiveActors();
  EXPECT_EQ(actors.size(), 3);
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
  // Pkmn 1: 6 moves + 2 swaps = 8 actions
  EXPECT_EQ(maps.size(), 64);  // 8 * 8
}


TEST_F(IsValidAction2v2Test, FaintedActiveRequiresWaitAndSwap) {
  auto fainted = setup_faintedTA1();
  auto faintedState = fainted.where1();

  // clang-format off
  // no pokemon can move if at least one pokemon has fainted:
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 0), Action::move(0)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 1), Action::move(0)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_B, 0), Action::move(0)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_B, 1), Action::move(0)));

  // Inactive pokemon are permitted to enter.
  EXPECT_FALSE(  // currenty in play
      engine_->isValidAction(faintedState, Actor(TEAM_A, 0), Action::activate()));
  EXPECT_FALSE(  // self, fainted
      engine_->isValidAction(faintedState, Actor(TEAM_A, 1), Action::activate()));
  EXPECT_TRUE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 2), Action::activate()));
  EXPECT_TRUE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 3), Action::activate()));

  // All other pokemon must wait.
  EXPECT_TRUE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 0), Action::wait()));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 1), Action::wait()));
  EXPECT_TRUE(
      engine_->isValidAction(faintedState, Actor(TEAM_B, 0), Action::wait()));
  EXPECT_TRUE(
      engine_->isValidAction(faintedState, Actor(TEAM_B, 1), Action::wait()));

  // no pokemon can swap swap.
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 0), Action::swap(2)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 1), Action::swap(2)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_B, 0), Action::swap(2)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_B, 1), Action::swap(2)));
  // clang-format on
}


TEST_F(IsValidAction2v2Test, FaintedActiveButCannotSwap) {
  auto fainted = setup_2v2_faintedTA1();
  auto faintedState = fainted.where1();

  // Actor(TEAM_A, 1) is fainted but cannot swap.
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 1), Action::swap(0)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 1), Action::swap(1)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 1), Action::swap(2)));
  EXPECT_FALSE(
      engine_->isValidAction(faintedState, Actor(TEAM_A, 1), Action::swap(3)));
}
