#include "engine_test.hpp"

#include <set>

class RoarTest : public Gen4EngineTest {
 protected:
  void SetUp() override { Gen4EngineTest::SetUp(); }

  TeamNonVolatile teamA() {
    auto team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("aerodactyl"))
            .addMove(pokedex_->move("roar"))
            .setLevel(100));
    return team_a;
  }

  TeamNonVolatile teamB2() {
    auto team_b_2 = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("swift"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle"))
          .setLevel(100));
    return team_b_2;
  }

  PossibleEnvironments setupStandard() {
    // Standard Roar Setup (2 pokemon on Team B)
    engine_->setEnvironment(EnvironmentNonvolatile(teamA(), teamB2(), true));

    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments setupTurn2() {
    engine_->setEnvironment(EnvironmentNonvolatile(teamA(), teamB2(), true));

    // Wait then Roar setup
    auto turn1_wait = engine_->updateState(
        engine_->initialState(), Action::wait(), Action::wait());
    return engine_->updateState(
        turn1_wait.where1().getEnv(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupBranch() {
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
    engine_->setEnvironment(EnvironmentNonvolatile(teamA(), team_b_3, true));
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupFail() {
    // Single Pokemon Setup (Should fail to switch)
    // clang-format off
    auto team_b_1 = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .setLevel(100));
    // clang-format on
    engine_->setEnvironment(EnvironmentNonvolatile(teamA(), team_b_1, true));
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupWhirlwind() {
    // Whirlwind Setup
    // clang-format off
    auto team_a_whirlwind = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("aerodactyl"))
          .addMove(pokedex_->move("whirlwind"))
          .setLevel(100));
    // clang-format on
    engine_->setEnvironment(
        EnvironmentNonvolatile(team_a_whirlwind, teamB2(), true));
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupHazards() {
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
        EnvironmentNonvolatile(team_a_hazards, teamB2(), true));
    auto setup_sr = engine_->updateState(
        engine_->initialState(), Action::move(1), Action::wait());
    return engine_->updateState(
        setup_sr.where1Hit(0).getEnv(), Action::move(0), Action::wait());
  }
};


TEST_F(RoarTest, ForcesSwitch) {
  auto roar_standard = setupStandard();
  auto state = roar_standard.where1Hit(0);
  // Verify charmander (idx 0) is swapped out for squirtle (idx 1)
  EXPECT_EQ(state.getTeam(1).getICPKV(), 1);

  // Verify Aerodactyl took damage (proving swift was NOT preempted due to
  // Roar's low priority)
  EXPECT_LT(
      state.teammate(0, 0).getHP(),
      state.teammate(0, 0).nv().getMaxHP());

  // Verification of hit symbols
  EXPECT_TRUE(state.flagsFor(TEAM_A).isHit());
  EXPECT_TRUE(state.flagsFor(TEAM_B).isHit());
}


TEST_F(RoarTest, FailsIfNoSwitch) {
  auto roar_fail = setupFail();
  auto state = roar_fail.where1Hit(0);
  // Verify charmander is still there
  EXPECT_EQ(state.getTeam(1).getICPKV(), 0);
}


TEST_F(RoarTest, BranchesIfMultipleOptions) {
  auto roar_branch = setupBranch();
  size_t hitCount = 0;
  std::set<size_t> switchedIndices;

  for (size_t i = 0; i < roar_branch.size(); ++i) {
    auto env = roar_branch.at(i);
    if (env.flagsFor(TEAM_A).isHit()) {  // Team A (0) hit
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
  auto whirlwind = setupWhirlwind();
  auto state = whirlwind.where1Hit(0);
  EXPECT_EQ(state.getTeam(1).getICPKV(), 1);
}


TEST_F(RoarTest, Turn2Roar) {
  auto roar_turn2 = setupTurn2();
  auto state = roar_turn2.where1Hit(0);
  EXPECT_EQ(state.getTeam(1).getICPKV(), 1);
}


TEST_F(RoarTest, TriggersHazards) {
  auto roar_hazards = setupHazards();
  auto state = roar_hazards.where1Hit(0);
  // Verify damage taken. Squirtle (Water) takes neutral damage from Rock
  // (12.5%). 1 - 0.125 = 0.875
  EXPECT_NEAR(state.teammate(1, 1).getPercentHP(), 0.875, 0.005);
}
