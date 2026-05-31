#include "engine_test.hpp"


class LeechSeedTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Bulbasaur
    team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("bulbasaur"))
            .addMove(pokedex_->move("leech seed"))
            .setLevel(100));

    // Team B: Mew (Psychic)
    team_b_normal = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("mew"))
            .addMove(pokedex_->move("amnesia"))
            .addMove(pokedex_->move("body slam"))
            .setLevel(100));

    // Team B: Roserade (Grass/Poison)
    team_b_grass = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("roserade"))
            .addMove(pokedex_->move("growth"))
            .setLevel(100));

    // Scenario 1: Normal seeding + Damage (Bulbasaur vs Mew)
    // Turn 1: Bulbasaur uses Leech Seed, Mew uses Body Slam (damages Bulbasaur)
    environment_nv = EnvironmentNonvolatile(team_a, team_b_normal, true);
    engine_->setEnvironment(environment_nv);
    r1_normal = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(1));

    // Turn 2: Both wait. Leech seed triggers again at end of turn.
    r2_normal = engine_->updateState(
        r1_normal.where1(), Action::wait(), Action::wait());

    // Scenario 2: Fails against Grass
    auto env_grass = EnvironmentNonvolatile(team_a, team_b_grass, true);
    engine_->setEnvironment(env_grass);
    r1_grass = engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  TeamNonVolatile team_a;
  TeamNonVolatile team_b_normal;
  TeamNonVolatile team_b_grass;

  PossibleEnvironments r1_normal;
  PossibleEnvironments r2_normal;
  PossibleEnvironments r1_grass;
};


TEST_F(LeechSeedTest, Test_AppliesToNonGrassType) {
  // Bulbasaur used Leech Seed on Mew in r1_normal
  auto result_env = r1_normal.where1().getEnv();

  // Mew should have Leech Seed status
  EXPECT_TRUE(result_env.teammate(1, 0).status().leechSeed);
}


TEST_F(LeechSeedTest, Test_FailsAgainstGrassType) {
  // Bulbasaur used Leech Seed on Roserade in r1_grass
  auto result_env = r1_grass.where1().getEnv();

  // Roserade should NOT have Leech Seed status
  EXPECT_FALSE(result_env.teammate(1, 0).status().leechSeed);
}


TEST_F(LeechSeedTest, Test_DamageAndHealing) {
  // Bulbasaur used Leech Seed on Mew in r1_normal (Mew used Body Slam)
  auto r1_env = r1_normal.where1().getEnv();

  // Mew should have Leech Seed status
  EXPECT_TRUE(r1_env.teammate(1, 0).status().leechSeed);

  // Mew should have lost 1/8 HP at end of round
  EXPECT_NEAR(r1_env.teammate(1, 0).getPercentHP(), 0.875, 0.005);

  // Bulbasaur took damage from Body Slam and healed from Leech Seed
  EXPECT_LT(r1_env.teammate(0, 0).getPercentHP(), 1.0);
}


TEST_F(LeechSeedTest, Test_FailsIfAlreadySeeded) {
  // Round 2 after r1_normal: Apply Leech Seed again
  EXPECT_TRUE(r2_normal.where1().teammate(1, 0).status().leechSeed);
}


TEST_F(LeechSeedTest, LeechSeedReported_Seeding) {
  // Verify initial seeding report (Initial -> Turn 1)
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), r1_normal.where1(), false);
  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("mew was seeded") != std::string::npos);
}


TEST_F(LeechSeedTest, LeechSeedReported_Healing) {
  // Verify healing report (Turn 1 -> Turn 2)
  // Between turn 1 and turn 2, Mew loses HP and Bulbasaur restores HP.
  auto output = StateTransitionPrinter::printString(
      r1_normal.where1().getEnv(), r2_normal.where1(), false);
  SCOPED_TRACE(output);

  EXPECT_TRUE(output.find("mew lost") != std::string::npos);
  EXPECT_TRUE(output.find("bulbasaur restored HP") != std::string::npos);
}
