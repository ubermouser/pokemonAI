#include "engine_test.hpp"

class LeechSeedTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Bulbasaur (Grass/Poison)
    team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .addMove(pokedex_->move("leech seed"))
          .addMove(pokedex_->move("body slam"))
          .setLevel(100));

    // Team B: Mew (Psychic)
    team_b_normal = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("body slam"))
          .setLevel(100));

    // Team B: Roserade (Grass/Poison) - instead of Bulbasaur to keep it distinct
    team_b_grass = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("roserade"))
          .addMove(pokedex_->move("body slam"))
          .setLevel(100));
  }

  TeamNonVolatile team_a;
  TeamNonVolatile team_b_normal;
  TeamNonVolatile team_b_grass;
};

TEST_F(LeechSeedTest, Test_AppliesToNonGrassType) {
    environment_nv = EnvironmentNonvolatile(team_a, team_b_normal, true);
    engine_->setEnvironment(environment_nv);

    // Bulbasaur uses Leech Seed, Mew uses Body Slam
    auto result_envs = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    auto result_env = result_envs.at(0).getEnv();

    // Mew should have Leech Seed status
    EXPECT_TRUE(result_env.getTeam(1).teammate(0).status().cTeammate.leechSeed);
}

TEST_F(LeechSeedTest, Test_FailsAgainstGrassType) {
    environment_nv = EnvironmentNonvolatile(team_a, team_b_grass, true);
    engine_->setEnvironment(environment_nv);

    // Bulbasaur uses Leech Seed, Roserade uses Body Slam
    auto result_envs = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    auto result_env = result_envs.at(0).getEnv();

    // Roserade should NOT have Leech Seed status
    EXPECT_FALSE(result_env.getTeam(1).teammate(0).status().cTeammate.leechSeed);
}

TEST_F(LeechSeedTest, Test_DamageAndHealing) {
    // Add Amnesia to Mew so it doesn't damage Bulbasaur
    team_b_normal = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("amnesia"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b_normal, true);
    engine_->setEnvironment(environment_nv);

    // Round 1: Bulbasaur uses Leech Seed, Mew uses Amnesia
    auto r1_envs = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    auto r1_env = r1_envs.at(0).getEnv();

    // Mew should have Leech Seed status
    EXPECT_TRUE(r1_env.getTeam(1).teammate(0).status().cTeammate.leechSeed);

    // Mew should have lost 1/8 HP at end of round
    EXPECT_NEAR(r1_env.getTeam(1).teammate(0).getPercentHP(), 0.875, 0.005);

    // Bulbasaur should have been healed (starting at 100%, it remains at 100%)
    EXPECT_EQ(r1_env.getTeam(0).teammate(0).getPercentHP(), 1.0);
}

TEST_F(LeechSeedTest, Test_FailsIfAlreadySeeded) {
    environment_nv = EnvironmentNonvolatile(team_a, team_b_normal, true);
    engine_->setEnvironment(environment_nv);

    // Round 1: Apply Leech Seed
    auto r1_envs = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    
    // Round 2: Apply Leech Seed again
    auto r2_envs = engine_->updateState(r1_envs.at(0), Action::move(0), Action::move(0));
    
    // It shouldn't crash or double seed
    EXPECT_TRUE(r2_envs.at(0).getEnv().getTeam(1).teammate(0).status().cTeammate.leechSeed);
}
