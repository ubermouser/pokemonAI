
#include "engine_test.hpp"

class FacadeTest : public EngineTest {
protected:
  uint32_t damage_normal;

  void SetUp() override {
    EngineTest::SetUp();

    // Pokemon with Facade
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("ursaring")) // Ursaring is a good Facade user
          .addMove(pokedex_->move("facade"))
          .addMove(pokedex_->move("faint attack")) // Control move
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    // Target Pokemon
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    // Compute damage_normal
    auto result_normal = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
    damage_normal = result_normal.at(0).getEnv().getTeam(1).getPKV().getMissingHP();
  }

  // Helper to get environment data with user having a specific status
  EnvironmentVolatileData getDataWithStatus(uint32_t status) {
    auto initial_state = engine_->initialState();
    auto env_data = initial_state.data();

    // Set status on team 0, pokemon 0 (active)
    env_data.teams[0].teammates[0].status_nonvolatile = status;

    return env_data;
  }
};

TEST_F(FacadeTest, NormalPower) {
  auto initial_env = engine_->initialState();
  auto result = engine_->updateState(initial_env, Action::move(0), Action::wait());
  auto result_env = result.at(0).getEnv();

  auto target_hp = result_env.getTeam(1).getPKV().getHP();
  auto max_hp = result_env.getTeam(1).getPKV().nv().getMaxHP();

  // Just ensure damage is done
  EXPECT_LT(target_hp, max_hp);

  // We can calculate expected damage if needed, but relative comparison is better.
}

TEST_F(FacadeTest, BoostedPowerBurn) {
  auto env_data = getDataWithStatus(AIL_NV_BURN);
  EnvironmentVolatile burned_env(engine_->initialState().nv(), env_data);

  // Calculate damage for burned Facade
  auto result_burned = engine_->updateState(burned_env, Action::move(0), Action::wait());
  auto damage_burned = result_burned.at(0).getEnv().getTeam(1).getPKV().getMissingHP();

  // Burn usually halves attack. Facade ignores burn reduction AND doubles power.
  // So effective power is 140 vs 70.
  // TODO: Implement burn damage reduction in the engine.
  // If burn reduction is NOT implemented in engine (as we found),
  // then normal burned damage (without Facade boost) would be same as normal damage.
  // With Facade boost (double power), damage should be ~2x normal damage.

  EXPECT_GT(damage_burned, damage_normal * 1.8);
  EXPECT_LT(damage_burned, damage_normal * 2.2);
}

TEST_F(FacadeTest, BoostedPowerParalysis) {
  auto env_data = getDataWithStatus(AIL_NV_PARALYSIS);
  EnvironmentVolatile paralyzed_env(engine_->initialState().nv(), env_data);

  auto result_paralyzed = engine_->updateState(paralyzed_env, Action::move(0), Action::wait());
  auto damage_paralyzed = result_paralyzed.at(0).getEnv().getTeam(1).getPKV().getMissingHP();

  EXPECT_GT(damage_paralyzed, damage_normal * 1.8);
  EXPECT_LT(damage_paralyzed, damage_normal * 2.2);
}

TEST_F(FacadeTest, BoostedPowerPoison) {
  auto env_data = getDataWithStatus(AIL_NV_POISON);
  EnvironmentVolatile poisoned_env(engine_->initialState().nv(), env_data);

  auto result_poisoned = engine_->updateState(poisoned_env, Action::move(0), Action::wait());
  auto damage_poisoned = result_poisoned.at(0).getEnv().getTeam(1).getPKV().getMissingHP();

  EXPECT_GT(damage_poisoned, damage_normal * 1.8);
  EXPECT_LT(damage_poisoned, damage_normal * 2.2);
}

TEST_F(FacadeTest, BoostedPowerToxic) {
  auto env_data = getDataWithStatus(AIL_NV_POISON_TOXIC);
  EnvironmentVolatile toxic_env(engine_->initialState().nv(), env_data);

  auto result_toxic = engine_->updateState(toxic_env, Action::move(0), Action::wait());
  auto damage_toxic = result_toxic.at(0).getEnv().getTeam(1).getPKV().getMissingHP();

  EXPECT_GT(damage_toxic, damage_normal * 1.8);
  EXPECT_LT(damage_toxic, damage_normal * 2.2);
}
