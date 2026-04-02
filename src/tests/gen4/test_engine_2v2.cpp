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


class IsValidAction2v2Test : public BasicEngine2v2Test {};


TEST_F(IsValidAction2v2Test, TargetsNothing) {
  // A targeted move without a target hits the directly opposite pokemon
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


TEST_F(IsValidAction2v2Test, AllFieldTargeted) {
  EXPECT_FALSE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 1), Action::moveEnemy(2, 0)));
}


TEST_F(IsValidAction2v2Test, AllField) {
  EXPECT_TRUE(engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 1), Action::moveAll(2)));
}


TEST_F(IsValidAction2v2Test, Activates2PokemonPerTeam) {
  auto actors = engine_->initialState().getActiveActors();
  EXPECT_EQ(actors.size(), 4);
}


TEST_F(IsValidAction2v2Test, AllValidActionMaps) {
  auto maps = engine_->getAllValidActions(engine_->initialState(), TEAM_A);

  fmt::print("{}\n", fmt::streamed(maps));
  for (const auto& map : maps) {
    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.count(Actor(TEAM_A, 0)));
    EXPECT_TRUE(map.count(Actor(TEAM_A, 1)));
  }

  // targeted attack: 2 enemies, 1 adjacent.
  // self attack: 1
  // adjacent attack: 1
  // all-field attack: 1
  // swap: 1
  EXPECT_EQ(
      maps.size(),
      (3 + 1 + 1 + 1 + 1) * (3 + 1 + 1 + 1));  // count may be incorrect
}


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
