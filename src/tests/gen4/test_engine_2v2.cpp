#include <stdexcept>

#include "engine_test.hpp"


class BasicEngine2v2Test : public Gen4EngineTest {
 protected:
  void SetUp() override {
#if USE_LEGACY_ENGINE
    GTEST_SKIP() << "Neo-Engine test";
#endif


    Gen4EngineTest::SetUp();
    pokedex_->setAllowInvalidPokemon(true);

    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("charmander"))
            .addMove(pokedex_->move("cut"))  // targeted attack
            .addMove(pokedex_->move("heat wave"))  // adjacent attack
            .addMove(pokedex_->move("rock slide"))  // all-field attack
            .addMove(pokedex_->move("swords dance"))  // self target
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("squirtle"))
            .addMove(pokedex_->move("aqua tail"))  // targeted attack
            .addMove(pokedex_->move("icy wind"))  // adjacent attack
            .addMove(pokedex_->move("surf"))  // all-field attack
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("bulbasaur"))
            .addMove(pokedex_->move("cut")));
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


TEST_F(BasicEngine2v2Test, TargetedMovesAgainstSwap) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveEnemy(0, 1)},
       {Actor(TEAM_A, 1), Action::moveEnemy(0, 1)}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::swap(2)}}
  );
  // clang-format on

  result.printStates();

  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_B, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 2).getMissingHP(), 0);
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


TEST_F(BasicEngine2v2Test, TargetedAdjacentMove) {
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

  auto state = result.where1Hit(0);
  EXPECT_GT(state.teammate(TEAM_A, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, TwoAdjacentMoves) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAdjacent(1)},
       {Actor(TEAM_A, 1), Action::moveAdjacent(0)}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 9);  // each pkmn does damage to 2 enemies

  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, AllFieldMove) {
  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::wait()},
       {Actor(TEAM_A, 1), Action::moveAll(2)}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 5);  // hits 3 pkmn

  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_A, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, HighEngineAccuracyTwoMoves) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);

  // clang-format off
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveAll(2)},
       {Actor(TEAM_A, 1), Action::moveAll(2)}},
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
      {{Actor(TEAM_A, 0), Action::moveAll(2)},
       {Actor(TEAM_A, 1), Action::moveAll(2)}},
      {{Actor(TEAM_B, 0), Action::moveAll(2)},
       {Actor(TEAM_B, 1), Action::moveAll(2)}}
  );
  // clang-format on

  result.printStates();
  EXPECT_EQ(result.size(), 16900);       // count is incorrect!
  EXPECT_EQ(result.getNumUnique(), 49);  // count is incorrect!
}
