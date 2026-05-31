#include "engine_test.hpp"

class ParalysisStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Mew (Speed 100)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("thunder wave"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    // Team B: Gengar (Speed 110)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("shadow ball"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupParalysisApplied() {
    return engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments setupParalyzedMoveTurn() {
    auto results = setupParalysisApplied();
    return engine_->updateState(
        results.where1Status(0).getEnv(),
        Action::move(1),
        Action::move(0));
  }

  PossibleEnvironments setupFullParalysisCheck() {
    auto results = setupParalysisApplied();
    return engine_->updateState(
        results.where1Status(0).getEnv(),
        Action::wait(),
        Action::move(0));
  }
};

TEST_F(ParalysisStatusTest, Test_AppliesParalysis) {
  auto results = setupParalysisApplied();

  // Gengar should be paralyzed in the state where Thunder Wave hit
  // Thunder Wave has 100% accuracy, so it should be the most probable state
  auto paralyzed_state = results.where1Status(0);
  EXPECT_EQ(paralyzed_state.teammate(1, 0).getStatusAilment(), AIL_NV_PARALYSIS);
}

TEST_F(ParalysisStatusTest, Test_ParalysisReducesSpeed) {
  // Turn 1: Mew uses Thunder Wave, Gengar uses Shadow Ball
  // Gengar (110) is faster than Mew (100), so Gengar should move first
  auto results1 = setupParalysisApplied();
  auto state1 = results1.where1Status(0);
  EXPECT_TRUE(state1.flagsFor(TEAM_B).isMovedFirst());
  EXPECT_EQ(state1.teammate(1, 0).getStatusAilment(), AIL_NV_PARALYSIS);

  // Turn 2: Mew uses Psychic, Gengar uses Shadow Ball
  // Now Gengar is paralyzed, so Mew (100) should be faster than Gengar (110/4 = 27.5)
  auto results2 = setupParalyzedMoveTurn();

  // Mew should move first in the most probable state where it hits
  auto state2 = results2.where1Hit(0);
  EXPECT_TRUE(state2.flagsFor(TEAM_A).isMovedFirst());
}

TEST_F(ParalysisStatusTest, Test_FullParalysis) {
  // Turn 2: Gengar tries to move (Shadow Ball), Mew waits
  auto results2 = setupFullParalysisCheck();

  // There should be a state where Gengar is blocked (25% chance)
  auto blocked_states = results2.where([](const ConstEnvironmentPossible& env) {
    return env.flagsFor(TEAM_B).isBlocked();
  });
  EXPECT_FALSE(blocked_states.empty());
  for (const auto& env : blocked_states) {
    EXPECT_NEAR(env.getProbability().to_double(), 0.25, 0.01);
  }

  auto not_blocked_states = results2.where([](const ConstEnvironmentPossible& env) {
    return !env.flagsFor(TEAM_B).isBlocked();
  });
  EXPECT_FALSE(not_blocked_states.empty());
}

TEST_F(ParalysisStatusTest, Test_StateTransitionPrinterParalysis) {
  auto results = setupParalysisApplied();
  auto paralyzed_state = results.where1Status(0);
  std::string output = StateTransitionPrinter::printString(
      engine_->initialState(),
      paralyzed_state,
      /*withStyle=*/false);
  EXPECT_TRUE(output.find("is paralyzed!") != std::string::npos)
      << "Expected 'is paralyzed!' in printer output: " << output;
}
