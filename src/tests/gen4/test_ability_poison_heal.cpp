#include "engine_test.hpp"

class PoisonHealTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Shroomish with Poison Heal
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("shroomish"))
          .setAbility(pokedex_->ability("poison heal"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    // Team B: Magikarp (Dummy)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magikarp"))
          .addMove(pokedex_->move("splash"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  EnvironmentNonvolatile environment_nv;
};

TEST_F(PoisonHealTest, PoisonHealRestoresHPWhenPoisoned) {
    auto c_env_v = engine_->initialState();
    EnvironmentVolatileData env_data = c_env_v.data();
    EnvironmentVolatile env_v(c_env_v.nv(), env_data);

    // Set Shroomish to 50% HP and Poisoned
    env_v.getTeam(TEAM_A).teammate(0).setPercentHP(0.5);
    env_v.getTeam(TEAM_A).teammate(0).setStatusAilment(AIL_NV_POISON);

    // Wait turn
    auto turn1 = engine_->updateState(env_v, Action::wait(), Action::wait());

    ASSERT_GE(turn1.size(), 1);
    auto pkv = turn1.where1().getTeam(TEAM_A).teammate(0);

    // Should heal 1/8 (12.5%) -> 62.5%
    // Tolerance for fixed point arithmetic
    EXPECT_NEAR(pkv.getPercentHP(), 0.625, 0.005);
}

TEST_F(PoisonHealTest, PoisonHealRestoresHPWhenToxicPoisoned) {
    auto c_env_v = engine_->initialState();
    EnvironmentVolatileData env_data = c_env_v.data();
    EnvironmentVolatile env_v(c_env_v.nv(), env_data);

    // Set Shroomish to 50% HP and Toxic Poisoned
    env_v.getTeam(TEAM_A).teammate(0).setPercentHP(0.5);
    env_v.getTeam(TEAM_A).teammate(0).setStatusAilment(AIL_NV_POISON_TOXIC);

    // Wait turn
    auto turn1 = engine_->updateState(env_v, Action::wait(), Action::wait());

    ASSERT_GE(turn1.size(), 1);
    auto pkv = turn1.where1().getTeam(TEAM_A).teammate(0);

    // Should heal 1/8 (12.5%) -> 62.5%
    EXPECT_NEAR(pkv.getPercentHP(), 0.625, 0.005);
}

TEST_F(PoisonHealTest, PoisonHealPreventsToxicCounterIncrease) {
    auto c_env_v = engine_->initialState();
    EnvironmentVolatileData env_data = c_env_v.data();
    EnvironmentVolatile env_v(c_env_v.nv(), env_data);

    // Set Shroomish to 50% HP and Toxic Poisoned
    env_v.getTeam(TEAM_A).teammate(0).setPercentHP(0.5);
    env_v.getTeam(TEAM_A).teammate(0).setStatusAilment(AIL_NV_POISON_TOXIC);

    // Check initial toxic tier (should be 0)
    EXPECT_EQ(env_v.getTeam(TEAM_A).teammate(0).status().cTeammate.toxicPoison_tier, 0);

    // Wait turn
    auto turn1 = engine_->updateState(env_v, Action::wait(), Action::wait());

    ASSERT_GE(turn1.size(), 1);
    auto pkv = turn1.where1().getTeam(TEAM_A).teammate(0);

    // Toxic tier should NOT increase (should remain 0)
    EXPECT_EQ(pkv.status().cTeammate.toxicPoison_tier, 0);
}

TEST_F(PoisonHealTest, NormalPoisonDamageWithoutAbility) {
     // Create a new environment where Squirtle has Torrent (implemented ability, not Poison Heal)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle"))
          .setAbility(pokedex_->ability("torrent"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    // Team B: Magikarp (Dummy)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magikarp"))
          .addMove(pokedex_->move("splash"))
          .setLevel(100));

    auto env_no_ph = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(env_no_ph);

    auto c_env_v = engine_->initialState();
    EnvironmentVolatileData env_data = c_env_v.data();
    EnvironmentVolatile env_v(c_env_v.nv(), env_data);

    env_v.getTeam(TEAM_A).teammate(0).setPercentHP(0.5);
    env_v.getTeam(TEAM_A).teammate(0).setStatusAilment(AIL_NV_POISON);

    auto turn1 = engine_->updateState(env_v, Action::wait(), Action::wait());
    ASSERT_GE(turn1.size(), 1);
    auto pkv = turn1.where1().getTeam(TEAM_A).teammate(0);

    // Should take damage 1/8 (12.5%) -> 37.5%
    EXPECT_NEAR(pkv.getPercentHP(), 0.375, 0.005);
}
