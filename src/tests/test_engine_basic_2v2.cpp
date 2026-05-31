#include "mock_engine_test.hpp"


class BasicEngine2v2Test : public MockEngineTest {
 protected:
  void SetUp() override {
    MockEngineTest::SetUp();
    plugin_calls.fill(0);

    // clang-format off
    auto teamA = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .addMove(pokedex_->move("move_any_adjacent"))
            .addMove(pokedex_->move("move_all_adjacent_enemy"))
            .addMove(pokedex_->move("move_all_adjacent"))
            .addMove(pokedex_->move("move_confuse_self"))
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon2"))
            .setIV(FV_SPEED, 10)  // enforce no speed tie
            .addMove(pokedex_->move("move_any_adjacent"))
            .addMove(pokedex_->move("move_all_adjacent_enemy"))
            .addMove(pokedex_->move("move_all_adjacent"))
            .addMove(pokedex_->move("move_suicide"))
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon3"))
            .setLevel(100));
    auto teamB = teamA;
    teamB.teammate(0).setIV(FV_SPEED, 5);
    teamB.teammate(1).setIV(FV_SPEED, 15);
    // clang-format on
    environment_nv = EnvironmentNonvolatile(teamA, teamB, true);
    engine_->setNumActivePokemon(2);
    engine_->setEnvironment(environment_nv);
  }

  ConstEnvironmentVolatile setup_2v2() {
    // clang-format off
    auto teamA = TeamNonVolatile()
        .addPokemon(environment_nv.teammate(TEAM_A, 0))
        .addPokemon(environment_nv.teammate(TEAM_A, 1));
    auto teamB = TeamNonVolatile()
        .addPokemon(environment_nv.teammate(TEAM_B, 0))
        .addPokemon(environment_nv.teammate(TEAM_B, 1));
    // clang-format on

    auto environment_2v2 = EnvironmentNonvolatile(teamA, teamB, true);
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


TEST_F(BasicEngine2v2Test, InsufficientActions) {
  // clang-format off
  EXPECT_THROW(engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::moveEnemy(0, 1)},
      {Actor(TEAM_B, 0), Action::wait()}}
  ), std::invalid_argument);
  // clang-format on
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
  EXPECT_EQ(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 1).getMissingHP(), 0);
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
  EXPECT_EQ(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_B, 1).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
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

  // move_all_adjacent_enemy hits 2. each can hit/miss/crit. 9 outcomes.
  result.printStates();
  EXPECT_EQ(result.size(), 9);

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
  // hits 3 pkmn. 3^3 = 27.
  EXPECT_EQ(result.size(), 27);
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
  EXPECT_EQ(result.size(), 3 * 3);  // each pkmn can miss, hit, and crit

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
  EXPECT_EQ(result.size(), 3 * 3);  // each pkmn can miss, hit, and crit

  auto state = result.where1Hit(0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 1).getMissingHP(), 0);
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
  EXPECT_EQ(result.size(), 3 * 3 * 3 * 3);
  auto state = result.where1Hit(0);
  EXPECT_EQ(state.teammate(TEAM_A, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(TEAM_A, 1).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
}


TEST_F(BasicEngine2v2Test, SecondToLastPokemonFaints) {
  auto state_2v2 = setup_2v2();

  auto result = engine_->updateState(
      state_2v2,
      {{Actor(TEAM_A, 0), Action::wait()},
       {Actor(TEAM_A, 1), Action::move(3)},
       {Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}});

  result.printStates();
  auto result_state = result.where1().getEnv();

  EXPECT_EQ(result_state.numActivePokemon(), 3);
}


TEST_F(BasicEngine2v2Test, FaintedPokemonAcceptsNoAction) {
  auto fainted = setup_2v2_faintedTA1();
  auto faintedState = fainted.where1();

  // Team A now only has 1 active pokemon (Pkmn 0).
  // Team B still has 2 active pokemon.
  // Total expected actions: 3.
  EXPECT_NO_THROW(engine_->updateState(
      faintedState.getEnv(),
      {{Actor(TEAM_A, 0), Action::move(0)},
       {Actor(TEAM_B, 0), Action::move(0)},
       {Actor(TEAM_B, 1), Action::move(0)}}));
}


TEST_F(BasicEngine2v2Test, HighEngineAccuracy_TwoMoves_MonteCarlo) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);
  engine_->setMaxNumStates(1);
  engine_->setStateSelectMethod(PkCU::StateSelectMethod::RANDOM);

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
  EXPECT_LE(result.size(), 1);
}


TEST_F(BasicEngine2v2Test, DISABLED_HighEngineAccuracy_TwoMoves) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);
  engine_->setMaxNumStates(10000);
  engine_->setStateSelectMethod(PkCU::StateSelectMethod::RANDOM);

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
  EXPECT_LE(result.size(), 10000);
}


TEST_F(BasicEngine2v2Test, DISABLED_HighEngineAccuracy_FourMoves) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);
  engine_->setMaxNumStates(10000);
  engine_->setStateSelectMethod(PkCU::StateSelectMethod::RANDOM);

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
  EXPECT_LE(result.size(), 10000);
}


TEST_F(BasicEngine2v2Test, VolatileStatusIsolation) {
  // TEAM_A 0 (test_pokemon) uses move_confuse_self (move index 3)
  // TEAM_A 1 (test_pokemon2) waits
  // TEAM_B 0 & 1 wait
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      {{Actor(TEAM_A, 0), Action::move(3)},
       {Actor(TEAM_A, 1), Action::wait()}},
      {{Actor(TEAM_B, 0), Action::wait()},
       {Actor(TEAM_B, 1), Action::wait()}}
  );

  result.printStates();
  EXPECT_EQ(result.size(), 1);

  auto state = result.where1().getEnv();
  // Actor(TEAM_A, 0) should be confused
  EXPECT_GT(state.teammate(TEAM_A, 0).status().confused, 0);
  // Actor(TEAM_A, 1) should NOT be confused
  EXPECT_EQ(state.teammate(TEAM_A, 1).status().confused, 0);
}