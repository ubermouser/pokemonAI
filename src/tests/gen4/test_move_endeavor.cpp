#include "engine_test.hpp"

class EndeavorTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Setup Team A: Rattata with Endeavor
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("rattata"))
          .addMove(pokedex_->move("endeavor"))
          .setLevel(100));

    // Setup Team B: Snorlax (dummy)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(EndeavorTest, Success) {
  // Get initial state from engine
  auto initialState = engine_->initialState();
  EnvironmentVolatileData envData = initialState.data();
  EnvironmentVolatile env(initialState.nv(), envData);

  // Set User HP to 1 (F.E.A.R. style)
  env.teammate(0, 0).setHP(1);

  // Target HP is full
  uint32_t targetMaxHP = env.teammate(1, 0).nv().getMaxHP();
  env.teammate(1, 0).setHP(targetMaxHP);

  // Turn 1: Team A uses Endeavor (Move 0)
  auto result = engine_->updateState(
      env, Action::move(0), Action::wait());

  auto target_pkv = result.where1().teammate(1, 0);
  auto user_pkv = result.where1().teammate(0, 0);

  // Check Target HP equals User HP
  EXPECT_EQ(target_pkv.getHP(), 1);
  EXPECT_EQ(user_pkv.getHP(), 1);
}

TEST_F(EndeavorTest, FailHighHP) {
  auto initialState = engine_->initialState();
  EnvironmentVolatileData envData = initialState.data();
  EnvironmentVolatile env(initialState.nv(), envData);

  // Set User HP to something high (e.g. 100)
  env.teammate(0, 0).setHP(100);

  // Set Target HP to something lower (e.g. 50)
  env.teammate(1, 0).setHP(50);

  // Turn 1: Team A uses Endeavor (Move 0)
  auto result = engine_->updateState(
      env, Action::move(0), Action::wait());

  auto target_pkv = result.where1().teammate(1, 0);

  // Check Target HP did not change
  EXPECT_EQ(target_pkv.getHP(), 50);
}

TEST_F(EndeavorTest, FailEqualHP) {
  auto initialState = engine_->initialState();
  EnvironmentVolatileData envData = initialState.data();
  EnvironmentVolatile env(initialState.nv(), envData);

  // Set User HP to 50
  env.teammate(0, 0).setHP(50);

  // Set Target HP to 50
  env.teammate(1, 0).setHP(50);

  // Turn 1: Team A uses Endeavor (Move 0)
  auto result = engine_->updateState(
      env, Action::move(0), Action::wait());

  auto target_pkv = result.where1().teammate(1, 0);

  // Check Target HP did not change
  EXPECT_EQ(target_pkv.getHP(), 50);
}

TEST_F(EndeavorTest, Immunity) {
   // Setup Team A: Rattata with Endeavor
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("rattata"))
          .addMove(pokedex_->move("endeavor"))
          .setLevel(100));

    // Setup Team B: Gengar (Ghost type)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("shadow ball"))
          .setLevel(100));

    EnvironmentNonvolatile env_nv(team_a, team_b, true);
    // update engine environment
    engine_->setEnvironment(env_nv);

    auto initialState = engine_->initialState();
    EnvironmentVolatileData envData = initialState.data();
    EnvironmentVolatile env(initialState.nv(), envData);

    // Set User HP to 1
    env.teammate(0, 0).setHP(1);

    // Turn 1: Team A uses Endeavor
    auto result = engine_->updateState(
      env, Action::move(0), Action::wait());

    auto target_pkv = result.where1().teammate(1, 0);

    // Check Target HP did not change (full HP)
    EXPECT_EQ(target_pkv.getHP(), target_pkv.nv().getMaxHP());
}
