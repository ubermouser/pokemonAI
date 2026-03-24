#include "engine_test.hpp"

#include <set>

class RoarTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Standard Roar Setup (2 pokemon on Team B)
    // clang-format off
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("aerodactyl"))
            .addMove(pokedex_->move("roar"))
            .setLevel(100));
    auto team_b_2 = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("swift"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle"))
          .setLevel(100));
    // clang-format on

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_2, true));
    roar_standard = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(0));

    // Wait then Roar setup
    auto turn1_wait = engine_->updateState(
        engine_->initialState(), Action::wait(), Action::wait());
    roar_turn2 = engine_->updateState(
        turn1_wait.where1().getEnv(), Action::move(0), Action::wait());

    // Single Pokemon Setup (Should fail to switch)
    // clang-format off
    auto team_b_1 = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .setLevel(100));
    // clang-format on
    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_1, true));
    roar_fail = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());

    // Multiple Options Setup (Branches)
    // clang-format off
    auto team_b_3 = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .setLevel(100));
    // clang-format on
    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_3, true));
    roar_branch = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());

    // Whirlwind Setup
    // clang-format off
    auto team_a_whirlwind = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("aerodactyl"))
          .addMove(pokedex_->move("whirlwind"))
          .setLevel(100));
    // clang-format on
    engine_->setEnvironment(
        EnvironmentNonvolatile(team_a_whirlwind, team_b_2, true));
    whirlwind = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());

    // Hazards Setup
    // clang-format off
    auto team_a_hazards = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("aerodactyl"))
          .addMove(pokedex_->move("roar"))
          .addMove(pokedex_->move("stealth rock"))
          .setLevel(100));
    // clang-format on
    engine_->setEnvironment(
        EnvironmentNonvolatile(team_a_hazards, team_b_2, true));
    auto setup_sr = engine_->updateState(
        engine_->initialState(), Action::move(1), Action::wait());
    roar_hazards = engine_->updateState(
        setup_sr.where1Hit(0).getEnv(), Action::move(0), Action::wait());
  }

  PossibleEnvironments roar_standard;
  PossibleEnvironments roar_fail;
  PossibleEnvironments roar_branch;
  PossibleEnvironments whirlwind;
  PossibleEnvironments roar_turn2;
  PossibleEnvironments roar_hazards;
};

TEST_F(RoarTest, ForcesSwitch) {
  auto state = roar_standard.where1Hit(0);
  // Verify charmander (idx 0) is swapped out for squirtle (idx 1)
  EXPECT_EQ(state.getTeam(1).getICPKV(), 1);

  // Verify Aerodactyl took damage (proving swift was NOT preempted due to
  // Roar's low priority)
  EXPECT_LT(
      state.teammate(0, 0).getHP(),
      state.teammate(0, 0).nv().getMaxHP());

  // Verification of hit symbols
  EXPECT_TRUE(state.hasHit(0));
  EXPECT_TRUE(state.hasHit(1));
}

TEST_F(RoarTest, FailsIfNoSwitch) {
  auto state = roar_fail.where1Hit(0);
  // Verify charmander is still there
  EXPECT_EQ(state.getTeam(1).getICPKV(), 0);
}

TEST_F(RoarTest, BranchesIfMultipleOptions) {
  size_t hitCount = 0;
  std::set<size_t> switchedIndices;

  for (size_t i = 0; i < roar_branch.size(); ++i) {
    auto env = roar_branch.at(i);
    if (env.hasHit(0)) {  // Team A (0) hit
      hitCount++;
      switchedIndices.insert(env.getTeam(1).getICPKV());
      // Check probability
      // Should be 0.5 (since 2 options)
      EXPECT_NEAR((float)env.getProbability(), 0.5, 0.001);
    }
  }

  EXPECT_EQ(hitCount, 2);
  EXPECT_EQ(switchedIndices.size(), 2);
  EXPECT_TRUE(switchedIndices.count(1));
  EXPECT_TRUE(switchedIndices.count(2));
}

TEST_F(RoarTest, WhirlwindWorks) {
  auto state = whirlwind.where1Hit(0);
  EXPECT_EQ(state.getTeam(1).getICPKV(), 1);
}

TEST_F(RoarTest, Turn2Roar) {
  auto state = roar_turn2.where1Hit(0);
  EXPECT_EQ(state.getTeam(1).getICPKV(), 1);
}

TEST_F(RoarTest, TriggersHazards) {
  auto state = roar_hazards.where1Hit(0);
  // Verify damage taken. Squirtle (Water) takes neutral damage from Rock
  // (12.5%). 1 - 0.125 = 0.875
  EXPECT_NEAR(state.teammate(1, 1).getPercentHP(), 0.875, 0.005);
}
