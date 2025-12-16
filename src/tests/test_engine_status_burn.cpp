#include "engine_test.hpp"

class BurnStatusTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

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

    // Initialize states
    burn_state = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    baseline_physical_state = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
    baseline_special_state = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(1));
    burned_physical_state = engine_->updateState(burn_state.at(0), Action::move(0), Action::move(0));
    burned_special_state = engine_->updateState(burn_state.at(0), Action::move(0), Action::move(1));

    burn_state_missing_hp = burn_state.at(0).getEnv().getTeam(1).teammate(0).getMissingHP();
    damage_on_mew_setup = burn_state.at(0).getEnv().getTeam(0).teammate(0).getMissingHP();
  }

  PossibleEnvironments burn_state;
  PossibleEnvironments baseline_physical_state;
  PossibleEnvironments burned_physical_state;
  PossibleEnvironments baseline_special_state;
  PossibleEnvironments burned_special_state;

  uint32_t burn_state_missing_hp;
  uint32_t damage_on_mew_setup;
};

TEST_F(BurnStatusTest, Test_AppliesBurn) {
    // Mew uses Will-o-wisp on Snorlax
    auto result_env = burn_state.at(0).getEnv();

    // Snorlax should be burned
    EXPECT_EQ(result_env.getTeam(1).teammate(0).getStatusAilment(), AIL_NV_BURN);
}

TEST_F(BurnStatusTest, Test_BurnDamage) {
    auto burned_env = burn_state.at(0).getEnv();
    
    // Verify initial full HP for Snorlax before burn damage trigger
    EXPECT_NEAR(burned_env.getTeam(1).teammate(0).getPercentHP(), 0.875, 0.005);
}

TEST_F(BurnStatusTest, Test_BurnReducesPhysicalDamage) {
    // Baseline: Snorlax uses Tackle on Mew (Clean state)
    uint32_t damage_baseline = baseline_physical_state.at(0).getEnv().getTeam(0).teammate(0).getMissingHP();

    // Burned: Mew uses Will-o-wisp, Snorlax uses Tackle
    uint32_t damage_burned_total = burned_physical_state.at(0).getEnv().getTeam(0).teammate(0).getMissingHP();
    uint32_t damage_burned = damage_burned_total - damage_on_mew_setup;

    // Burn reduces physical damage by 50%
    EXPECT_NEAR(damage_burned, damage_baseline / 2.0, 5.0);
    EXPECT_LT(damage_burned, damage_baseline);
}

TEST_F(BurnStatusTest, Test_BurnDoesNotReduceSpecialDamage) {
    // Baseline: Snorlax uses Psychic on Mew (Clean state)
    uint32_t damage_baseline = baseline_special_state.at(0).getEnv().getTeam(0).teammate(0).getMissingHP();

    // Burned: Mew uses Will-o-wisp, Snorlax uses Psychic
    uint32_t damage_burned_total = burned_special_state.at(0).getEnv().getTeam(0).teammate(0).getMissingHP();
    uint32_t damage_burned = damage_burned_total - damage_on_mew_setup;

    // Burn does NOT reduce special damage
    EXPECT_EQ(damage_burned, damage_baseline);
}
