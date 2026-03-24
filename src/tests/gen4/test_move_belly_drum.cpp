#include "engine_test.hpp"

class BellyDrumTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Setup Team A: Snorlax with Belly Drum
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("belly drum"))
          .setLevel(100));

    // Setup Team B: Magikarp (dummy)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magikarp"))
          .addMove(pokedex_->move("splash"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(BellyDrumTest, Success) {
  // Turn 1: Team A uses Belly Drum (Move 0)
  auto result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  auto pkv = result.where1().teammate(0, 0);

  // Check HP reduction (50% max HP)
  uint32_t maxHP = pkv.nv().getMaxHP();
  uint32_t currentHP = pkv.getHP();
  EXPECT_EQ(currentHP, maxHP - (maxHP / 2));

  // Check Attack boost (+6)
  EXPECT_EQ(pkv.getBoost(FV_ATTACK), 6);
}

TEST_F(BellyDrumTest, FailLowHP) {
  // Get initial state from engine to ensure NV match
  auto initialState = engine_->initialState();

  // Copy data
  EnvironmentVolatileData envData = initialState.data();

  // Create modifiable environment using ENGINE's NV
  EnvironmentVolatile env(initialState.nv(), envData);

  uint32_t maxHP = env.teammate(0, 0).nv().getMaxHP();
  // Set HP to 50%
  env.teammate(0, 0).setHP(maxHP / 2);

  auto result = engine_->updateState(
      env, Action::move(0), Action::wait());

  auto pkv_result = result.where1().teammate(0, 0);

  // Check HP did not change (move failed)
  EXPECT_EQ(pkv_result.getHP(), maxHP / 2);

  // Check Attack boost did not change (0)
  EXPECT_EQ(pkv_result.getBoost(FV_ATTACK), 0);
}

TEST_F(BellyDrumTest, FailMaxAtk) {
  // Get initial state from engine to ensure NV match
  auto initialState = engine_->initialState();

  // Copy data
  EnvironmentVolatileData envData = initialState.data();

  // Create modifiable environment using ENGINE's NV
  EnvironmentVolatile env(initialState.nv(), envData);

  // Set Attack to +6
  env.teammate(0, 0).setBoost(FV_ATTACK, 6);

  auto result = engine_->updateState(
      env, Action::move(0), Action::wait());

  auto pkv_result = result.where1().teammate(0, 0);

  // Check HP did not change (move failed)
  EXPECT_EQ(pkv_result.getHP(), env.teammate(0, 0).nv().getMaxHP());
}
