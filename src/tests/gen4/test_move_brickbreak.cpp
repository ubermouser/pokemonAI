#include "engine_test.hpp"


class BrickBreakTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("brick break"))
          .addMove(pokedex_->move("psychic")) // dummy move
          .addMove(pokedex_->move("reflect"))
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mewtwo"))
          .addMove(pokedex_->move("reflect"))
          .addMove(pokedex_->move("light screen"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("rotom"))
          .addMove(pokedex_->move("reflect"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupReflect() {
    return engine_->updateState(engine_->initialState(), Action::wait(), Action::moveSideAlly(0));
  }

  PossibleEnvironments setupLightscreen() {
    return engine_->updateState(engine_->initialState(), Action::wait(), Action::moveSideAlly(1));
  }

  PossibleEnvironments setupBoth() {
    return engine_->updateState(
        setupReflect().where1(), Action::wait(), Action::moveSideAlly(1));
  }

  PossibleEnvironments setupFriendlyReflect() {
    return engine_->updateState(engine_->initialState(), Action::moveSideAlly(2), Action::wait());
  }

  PossibleEnvironments setupReflectSwappedToGhost() {
    return engine_->updateState(
        setupReflect().where1(), Action::wait(), Action::swap(1));
  }
};

TEST_F(BrickBreakTest, RemovesReflect) {
  auto turn1 = setupReflect();
  auto setup_env = turn1.where1().getEnv();

  EXPECT_EQ(turn1.where1().getTeam(TEAM_B).status().reflect, 5);

  // Test: Team A uses Brick Break
  auto result = engine_->updateState(setup_env, Action::move(0), Action::wait());
  auto state = result.where1Hit(TEAM_A);

  // Reflect should be removed (set to 0)
  EXPECT_EQ(state.getTeam(TEAM_B).status().reflect, 0);
  // Damage should be dealt
  EXPECT_LT(state.teammate(TEAM_B, 0).getPercentHP(), 1.0);
}

TEST_F(BrickBreakTest, RemovesLightScreen) {
  auto turn1 = setupLightscreen();
  auto setup_env = turn1.where1().getEnv();

  EXPECT_EQ(setup_env.getTeam(TEAM_B).status().lightScreen, 5);

  // Test: Team A uses Brick Break
  auto result = engine_->updateState(setup_env, Action::move(0), Action::wait());
  auto state = result.where1Hit(TEAM_A);

  // Light Screen should be removed
  EXPECT_EQ(state.getTeam(TEAM_B).status().lightScreen, 0);
  // Damage should be dealt
  EXPECT_LT(state.teammate(TEAM_B, 0).getPercentHP(), 1.0);
}

TEST_F(BrickBreakTest, RemovesBothScreens) {
  auto turn2 = setupBoth();
  auto setup_env = turn2.where1().getEnv();

  EXPECT_EQ(setup_env.getTeam(TEAM_B).status().reflect, 4); // decremented once
  EXPECT_EQ(setup_env.getTeam(TEAM_B).status().lightScreen, 5);

  // Test: Team A uses Brick Break
  auto result = engine_->updateState(setup_env, Action::move(0), Action::wait());
  auto state = result.where1Hit(TEAM_A);

  EXPECT_EQ(state.getTeam(TEAM_B).status().reflect, 0);
  EXPECT_EQ(state.getTeam(TEAM_B).status().lightScreen, 0);
}

TEST_F(BrickBreakTest, WorksWithoutScreens) {
  // Test: Team A uses Brick Break without any screens on Team B
  auto result = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto state = result.where1Hit(TEAM_A);

  // Damage should still be dealt
  EXPECT_LT(state.teammate(TEAM_B, 0).getPercentHP(), 1.0);
}

TEST_F(BrickBreakTest, DoesNotRemoveUserScreens) {
  auto turn1 = setupFriendlyReflect();
  auto setup_env = turn1.where1().getEnv();
  EXPECT_EQ(setup_env.getTeam(TEAM_A).status().reflect, 5);

  // Team A uses Brick Break
  auto result = engine_->updateState(setup_env, Action::move(0), Action::wait());
  auto state = result.where1Hit(TEAM_A);

  // Team A's Reflect should remain (decremented to 4)
  EXPECT_EQ(state.getTeam(TEAM_A).status().reflect, 4);
}

TEST_F(BrickBreakTest, DoesNotRemoveScreensIfImmune) {
  auto turn2 = setupReflectSwappedToGhost();
  auto setup_env = turn2.where1().getEnv();
  // TODO(@drendleman) once lightscreen counter bug is solved, should be 4
  EXPECT_EQ(setup_env.getTeam(TEAM_B).status().reflect, 5);

  // Team A uses Brick Break on Rotom (Immune)
  auto result = engine_->updateState(setup_env, Action::move(0), Action::moveSideAlly(0));
  auto state = result.where1();

  // Reflect should REMAIN (decremented to 4) because Ghost is immune to Fighting
  EXPECT_EQ(state.getTeam(TEAM_B).status().reflect, 4);
  // No damage dealt
  EXPECT_EQ(state.teammate(TEAM_B, 1).getPercentHP(), 1.0);
}
