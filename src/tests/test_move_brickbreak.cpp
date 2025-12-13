#include "engine_test.hpp"


class BrickBreakTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

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

    setup_reflect = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
    setup_lightscreen = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(1));
    setup_both = engine_->updateState(setup_reflect.at(0), Action::wait(), Action::move(1));
    setup_friendly_reflect = engine_->updateState(engine_->initialState(), Action::move(2), Action::wait());
    setup_reflect_swapped_to_ghost = engine_->updateState(setup_reflect.at(0), Action::wait(), Action::swap(1));
  }

  PossibleEnvironments setup_reflect;
  PossibleEnvironments setup_lightscreen;
  PossibleEnvironments setup_both;
  PossibleEnvironments setup_friendly_reflect;
  PossibleEnvironments setup_reflect_swapped_to_ghost;
};

TEST_F(BrickBreakTest, RemovesReflect) {
  auto setup_env = setup_reflect.at(0).getEnv();

  EXPECT_EQ(setup_reflect.at(0).getEnv().getTeam(1).getNonVolatile().reflect, 5);

  // Test: Team A uses Brick Break
  auto result = engine_->updateState(setup_env, Action::move(0), Action::wait());
  auto result_env = result.at(0).getEnv();

  // Reflect should be removed (set to 0)
  EXPECT_EQ(result_env.getTeam(1).getNonVolatile().reflect, 0);
  // Damage should be dealt
  EXPECT_LT(result_env.getTeam(1).getPKV().getPercentHP(), 1.0);
}

TEST_F(BrickBreakTest, RemovesLightScreen) {
  auto setup_env = setup_lightscreen.at(0).getEnv();

  EXPECT_EQ(setup_env.getTeam(1).getNonVolatile().lightScreen, 5);

  // Test: Team A uses Brick Break
  auto result = engine_->updateState(setup_env, Action::move(0), Action::wait());
  auto result_env = result.at(0).getEnv();

  // Light Screen should be removed
  EXPECT_EQ(result_env.getTeam(1).getNonVolatile().lightScreen, 0);
  // Damage should be dealt
  EXPECT_LT(result_env.getTeam(1).getPKV().getPercentHP(), 1.0);
}

TEST_F(BrickBreakTest, RemovesBothScreens) {
  auto setup_env = setup_both.at(0).getEnv();

  EXPECT_EQ(setup_env.getTeam(1).getNonVolatile().reflect, 4); // decremented once
  EXPECT_EQ(setup_env.getTeam(1).getNonVolatile().lightScreen, 5);

  // Test: Team A uses Brick Break
  auto result = engine_->updateState(setup_env, Action::move(0), Action::wait());
  auto result_env = result.at(0).getEnv();

  EXPECT_EQ(result_env.getTeam(1).getNonVolatile().reflect, 0);
  EXPECT_EQ(result_env.getTeam(1).getNonVolatile().lightScreen, 0);
}

TEST_F(BrickBreakTest, WorksWithoutScreens) {
  // Test: Team A uses Brick Break without any screens on Team B
  auto result = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto result_env = result.at(0).getEnv();

  // Damage should still be dealt
  EXPECT_LT(result_env.getTeam(1).getPKV().getPercentHP(), 1.0);
}

TEST_F(BrickBreakTest, DoesNotRemoveUserScreens) {
  auto setup_env = setup_friendly_reflect.at(0).getEnv();
  EXPECT_EQ(setup_env.getTeam(0).getNonVolatile().reflect, 5);

  // Team A uses Brick Break
  auto result = engine_->updateState(setup_env, Action::move(0), Action::wait());
  auto result_env = result.at(0).getEnv();

  // Team A's Reflect should remain (decremented to 4)
  EXPECT_EQ(result_env.getTeam(0).getNonVolatile().reflect, 4);
}

TEST_F(BrickBreakTest, DoesNotRemoveScreensIfImmune) {
  auto setup_env = setup_reflect_swapped_to_ghost.at(0).getEnv();
  // TODO(@drendleman) once lightscreen counter bug is solved, should be 4
  EXPECT_EQ(setup_env.getTeam(1).getNonVolatile().reflect, 5);

  // Team A uses Brick Break on Rotom (Immune)
  auto result = engine_->updateState(setup_env, Action::move(0), Action::move(0));
  auto result_env = result.at(0).getEnv();

  // Reflect should REMAIN (decremented to 3) because Ghost is immune to Fighting
  EXPECT_EQ(result_env.getTeam(1).getNonVolatile().reflect, 4);
  // No damage dealt
  EXPECT_EQ(result_env.getTeam(1).getPKV().getPercentHP(), 1.0);
}
