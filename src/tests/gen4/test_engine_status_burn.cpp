#include "engine_test.hpp"

class BurnStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Mew (Psychic)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("will-o-wisp"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    // Team B: Snorlax (Normal)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("tackle"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupBurnApplied() {
    return engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments setupBaselinePhysical() {
    return engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments setupBaselineSpecial() {
    return engine_->updateState(engine_->initialState(), Action::wait(), Action::move(1));
  }

  PossibleEnvironments setupBurnedPhysical() {
    auto results = setupBurnApplied();
    return engine_->updateState(
        results.where1Status(0).getEnv(),
        Action::move(0),
        Action::move(0));
  }

  PossibleEnvironments setupBurnedSpecial() {
    auto results = setupBurnApplied();
    return engine_->updateState(
        results.where1Status(0).getEnv(),
        Action::move(0),
        Action::move(1));
  }
};

TEST_F(BurnStatusTest, Test_AppliesBurn) {
    // Mew uses Will-o-wisp on Snorlax
    auto results = setupBurnApplied();
    auto result_env = results.where1Status(0).getEnv();

    // Snorlax should be burned
    EXPECT_EQ(result_env.teammate(1, 0).getStatusAilment(), AIL_NV_BURN);
}

TEST_F(BurnStatusTest, Test_BurnDamage) {
  auto results = setupBurnApplied();
  auto burned_env = results.where1Status(0).getEnv();

  // Verify initial full HP for Snorlax before burn damage trigger
  EXPECT_NEAR(burned_env.teammate(1, 0).getPercentHP(), 0.875, 0.005);
}

TEST_F(BurnStatusTest, Test_BurnReducesPhysicalDamage) {
    // Baseline: Snorlax uses Tackle on Mew (Clean state)
    auto baseline = setupBaselinePhysical();
    uint32_t damage_baseline =
        baseline.where1Hit(1).teammate(0, 0).getMissingHP();

    // Burned: Mew uses Will-o-wisp, Snorlax uses Tackle
    auto burn_results = setupBurnApplied();
    auto burned_state = burn_results.where1Status(0);
    uint32_t damage_on_mew_setup = burned_state.teammate(0, 0).getMissingHP();

    auto burned = setupBurnedPhysical();
    uint32_t damage_burned_total =
        burned.where1Hit(1).teammate(0, 0).getMissingHP();
    uint32_t damage_burned = damage_burned_total - damage_on_mew_setup;

    // Burn reduces physical damage by 50%
    EXPECT_NEAR(damage_burned, damage_baseline / 2.0, 5.0);
    EXPECT_LT(damage_burned, damage_baseline);
}

TEST_F(BurnStatusTest, Test_BurnDoesNotReduceSpecialDamage) {
    // Baseline: Snorlax uses Psychic on Mew (Clean state)
    auto baseline = setupBaselineSpecial();
    uint32_t damage_baseline =
        baseline.where1Hit(1).teammate(0, 0).getMissingHP();

    // Burned: Mew uses Will-o-wisp, Snorlax uses Psychic
    auto burn_results = setupBurnApplied();
    auto burned_state = burn_results.where1Status(0);
    uint32_t damage_on_mew_setup = burned_state.teammate(0, 0).getMissingHP();

    auto burned = setupBurnedSpecial();
    uint32_t damage_burned_total =
        burned.where1Hit(1).teammate(0, 0).getMissingHP();
    uint32_t damage_burned = damage_burned_total - damage_on_mew_setup;

    // Burn does NOT reduce special damage
    EXPECT_EQ(damage_burned, damage_baseline);
}

TEST_F(BurnStatusTest, Test_StateTransitionPrinterBurn) {
    auto results = setupBurnApplied();
    auto burned_state = results.where1Status(0);
    std::string output = StateTransitionPrinter::printString(
        engine_->initialState(),
        burned_state,
        /*withStyle=*/false);
    EXPECT_TRUE(output.find("was burned!") != std::string::npos)
        << "Expected 'was burned!' in printer output: " << output;
}
