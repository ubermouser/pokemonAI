#include <set>

#include "engine_test.hpp"

class RoarTest : public Gen4EngineTest {
 protected:
  void SetUp() override { Gen4EngineTest::SetUp(); }

  // clang-format off
  TeamNonVolatile teamA() {
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
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
  // clang-format on

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
  EXPECT_TRUE(state.teammate(1, 1).isActive());

  // Verify Aerodactyl took damage (proving swift was NOT preempted due to
  // Roar's low priority)
  EXPECT_LT(state.teammate(0, 0).getHP(), state.teammate(0, 0).nv().getMaxHP());

  // Verification of hit symbols
  EXPECT_TRUE(state.flagsFor(TEAM_A).isHit());
  EXPECT_TRUE(state.flagsFor(TEAM_B).isHit());
}


TEST_F(RoarTest, FailsIfNoSwitch) {
  auto roar_fail = setupFail();
  auto state = roar_fail.where1Hit(0);
  // Verify charmander is still there
  EXPECT_TRUE(state.teammate(1, 0).isActive());
}


TEST_F(RoarTest, BranchesIfMultipleOptions_HitCount) {
  auto roar_branch = setupBranch();
  auto hitStates = roar_branch.whereHit(TEAM_A);
  EXPECT_EQ(hitStates.size(), 2);
}


TEST_F(RoarTest, BranchesIfMultipleOptions_Probability) {
  auto roar_branch = setupBranch();
  auto hitStates = roar_branch.whereHit(TEAM_A);
  for (const auto& env : hitStates) {
    EXPECT_NEAR((float)env.getProbability(), 0.5, 0.001);
  }
}


TEST_F(RoarTest, BranchesIfMultipleOptions_ActiveTeammates) {
  auto roar_branch = setupBranch();

  auto active0 = roar_branch.where([](const ConstEnvironmentPossible& env) {
    return env.teammate(TEAM_B, 0).isActive();
  });
  auto active1 = roar_branch.where([](const ConstEnvironmentPossible& env) {
    return env.teammate(TEAM_B, 1).isActive();
  });
  auto active2 = roar_branch.where([](const ConstEnvironmentPossible& env) {
    return env.teammate(TEAM_B, 2).isActive();
  });

  EXPECT_EQ(active0.size(), 0);
  EXPECT_EQ(active1.size(), 1);
  EXPECT_EQ(active2.size(), 1);
}


TEST_F(RoarTest, WhirlwindWorks) {
  auto whirlwind = setupWhirlwind();
  auto state = whirlwind.where1Hit(0);
  EXPECT_TRUE(state.teammate(1, 1).isActive());
}


TEST_F(RoarTest, Turn2Roar) {
  auto roar_turn2 = setupTurn2();
  auto state = roar_turn2.where1Hit(0);
  EXPECT_TRUE(state.teammate(1, 1).isActive());
}


TEST_F(RoarTest, TriggersHazards) {
  auto roar_hazards = setupHazards();
  auto state = roar_hazards.where1Hit(0);
  // Verify damage taken. Squirtle (Water) takes neutral damage from Rock
  // (12.5%). 1 - 0.125 = 0.875
  EXPECT_NEAR(state.teammate(1, 1).getPercentHP(), 0.875, 0.005);
}
