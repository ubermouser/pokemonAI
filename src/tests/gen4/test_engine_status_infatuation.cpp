#include "engine_test.hpp"

class InfatuationStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Mew (Male)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("attract"))
          .addMove(pokedex_->move("psychic"))
          .setSex(SEX_MALE)
          .setLevel(100));

    // Team B: Snorlax (Female)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("strength"))
          .setSex(SEX_FEMALE)
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupInfatuationApplied() {
    return engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupInfatuationTurn2() {
    auto results = setupInfatuationApplied();
    return engine_->updateState(
        results.where1Status(0).getEnv(),
        Action::wait(),
        Action::move(0));
  }
};

TEST_F(InfatuationStatusTest, Test_AppliesInfatuation) {
  auto results = setupInfatuationApplied();

  // Snorlax should be infatuated in the state where Attract hit
  auto infatuated_state = results.where1Status(0);
  EXPECT_EQ(infatuated_state.teammate(1, 0).status().infatuate, 1);
}

TEST_F(InfatuationStatusTest, Test_InfatuationBlocksMove) {
  auto results2 = setupInfatuationTurn2();

  // There should be a state where Snorlax is blocked (50% chance)
  auto blocked_states = results2.where([](const ConstEnvironmentPossible& env) {
    return env.flagsFor(TEAM_B).isBlocked();
  });
  EXPECT_FALSE(blocked_states.empty());
  for (const auto& env : blocked_states) {
    EXPECT_NEAR(env.getProbability().to_double(), 0.5, 0.01);
  }

  auto hit_states = results2.where([](const ConstEnvironmentPossible& env) {
    return env.flagsFor(TEAM_B).isHit();
  });
  EXPECT_FALSE(hit_states.empty());
  double prob_not_blocked = 0.0;
  for (const auto& env : hit_states) {
    prob_not_blocked += env.getProbability().to_double();
  }
  EXPECT_NEAR(prob_not_blocked, 0.5, 0.01);
}

TEST_F(InfatuationStatusTest, Test_StateTransitionPrinterInfatuation) {
  auto results = setupInfatuationApplied();
  auto infatuated_state = results.where1Status(0);
  std::string output = StateTransitionPrinter::printString(
      engine_->initialState(),
      infatuated_state,
      /*withStyle=*/false);
  EXPECT_TRUE(output.find("fell in love!") != std::string::npos)
      << "Expected 'fell in love!' in printer output: " << output;
}
