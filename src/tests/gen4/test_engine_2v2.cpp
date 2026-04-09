#include <stdexcept>

#include "engine_test.hpp"
#include "pokemonai/pkai.h"


class BasicEngine2v2Test : public Gen4EngineTest {
 protected:
  void SetUp() override {

    Gen4EngineTest::SetUp();
    pokedex_->setAllowInvalidPokemon(true);

    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("charmander"))
            .addMove(pokedex_->move("cut"))  // targeted attack
            .addMove(pokedex_->move("heat wave"))  // adjacent-enemy attack
            .addMove(pokedex_->move("earthquake"))  // all-adjacent attack
            .addMove(pokedex_->move("swords dance"))  // self target
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("squirtle"))
            .addMove(pokedex_->move("aqua tail"))  // targeted attack
            .addMove(pokedex_->move("icy wind"))  // adjacent-enemy attack
            .addMove(pokedex_->move("surf"))  // all-adjacent attack
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("bulbasaur"))
            .addMove(pokedex_->move("cut"))
            .setLevel(100));
    // clang-format on
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setNumActivePokemon(2);
    engine_->setEnvironment(environment);
  }
};


TEST_F(BasicEngine2v2Test, InsufficientActions) {
  EXPECT_THROW(
      engine_->updateState(
          engine_->initialState(), Action::move(0), Action::move(1)),
      std::invalid_argument);
}


TEST_F(BasicEngine2v2Test, TargetAdjacentEnemy) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveEnemy(0, 1)},
       {Actor(TEAM_A, 1), Action::wait()}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.getNumUnique(), 3);

  auto state = result.where1Hit(0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, TargetAdjacentAlly) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAlly(0, 1)},
       {Actor(TEAM_A, 1), Action::wait()}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.getNumUnique(), 3);

  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_B, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_A, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, AdjacentEnemy) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAdjacentEnemy(1)},
       {Actor(TEAM_A, 1), Action::wait()}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  // pkmn does damage to 2 enemies, 5 outcomes each
  result.printStates();
  EXPECT_EQ(result.size(), 25);

  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, AdjacentAll) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::wait()},
       {Actor(TEAM_A, 1), Action::moveAdjacent(2)}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 8);  // hits 3 pkmn, may hit or crit

  auto state = result.where1Hit(0);
  EXPECT_GT(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, TargetedMoveAgainstSwap) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveEnemy(0, 1)},
       {Actor(TEAM_A, 1), Action::wait()}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::swap(2)}}
  );
  // clang-format on

  result.printStates();

  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_B, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 2).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, AdjacentMoveAgainstSwap) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAdjacentEnemy(1)},
       {Actor(TEAM_A, 1), Action::wait()}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::swap(2)}}
  );
  // clang-format on

  result.printStates();

  auto state = result.where1Hit(0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_B, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 2).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, TwoTargetedMoves) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveEnemy(0, 1)},
       {Actor(TEAM_A, 1), Action::moveEnemy(0, 1)}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 9);  // each pkmn can miss, hit, and crit

  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 1).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, TwoTargetedDefaultMoves) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::move(0)},
       {Actor(TEAM_A, 1), Action::move(0)}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 9);  // each pkmn can miss, hit, and crit

  auto state = result.where1Hit(0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, TwoAdjacentEnemyMoves) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAdjacentEnemy(1)},
       {Actor(TEAM_A, 1), Action::moveAdjacentEnemy(1)}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  // each pkmn is hit twice.
  // First move can [miss, hit, crit, status, status-crit]
  // Second move can [miss, status, status-crit]
  EXPECT_EQ(result.size(), 5 * 3 * 5 * 3);

  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, HighEngineAccuracyTwoMoves) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);

  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAdjacent(2)},
       {Actor(TEAM_A, 1), Action::moveAdjacent(2)}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 110);         // count is incorrect!
  EXPECT_EQ(result.getNumUnique(), 14);  // count is incorrect!
}


TEST_F(BasicEngine2v2Test, HighEngineAccuracyFourMoves) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);

  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAdjacent(2)},
       {Actor(TEAM_A, 1), Action::moveAdjacent(2)}},
      {{Actor(TEAM_B, 0), Action::moveAdjacent(2)},
       {Actor(TEAM_B, 1), Action::moveAdjacent(2)}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 16900);       // count is incorrect!
  EXPECT_EQ(result.getNumUnique(), 49);  // count is incorrect!
}
