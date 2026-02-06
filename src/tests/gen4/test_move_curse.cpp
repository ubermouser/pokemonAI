#include "engine_test.hpp"

class CurseTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Gengar (Ghost/Poison) - User of Curse
    team_a_ghost = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("gengar"))
            .addMove(pokedex_->move("curse"))
            .setLevel(100));

    // Team A: Snorlax (Normal) - User of Curse
    team_a_normal = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("snorlax"))
            .addMove(pokedex_->move("curse"))
            .setLevel(100));

    // Team B: Snorlax (Normal) - Target
    team_b = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("snorlax"))
            .addMove(pokedex_->move("tackle")) // do nothing
            .setLevel(100));
  }

  TeamNonVolatile team_a_ghost;
  TeamNonVolatile team_a_normal;
  TeamNonVolatile team_b;
};

TEST_F(CurseTest, GhostCurse_AppliesStatusAndCutsHP) {
  // Gengar uses Curse on Snorlax
  auto env = EnvironmentNonvolatile(team_a_ghost, team_b, true);
  engine_->setEnvironment(env);

  auto r1 = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  auto r1_env = r1.where1().getEnv();

  // Gengar should have lost 1/2 max HP
  // Max HP of Gengar at lvl 100 ~261. 50% loss.
  EXPECT_NEAR(r1_env.teammate(0, 0).getPercentHP(), 0.5, 0.01);

  // Snorlax should have Curse status
  EXPECT_TRUE(r1_env.teammate(1, 0).status().cTeammate.curse);

  // Snorlax should have taken 1/4 max HP damage at end of round 1
  EXPECT_NEAR(r1_env.teammate(1, 0).getPercentHP(), 0.75, 0.01);

  // Round 2
  auto r2 = engine_->updateState(
      r1.where1(), Action::wait(), Action::wait());

  auto r2_env = r2.where1().getEnv();

  // Snorlax HP should be 0.50
  EXPECT_NEAR(r2_env.teammate(1, 0).getPercentHP(), 0.50, 0.01);
}

TEST_F(CurseTest, NonGhostCurse_BoostsStats) {
  // Snorlax uses Curse
  auto env = EnvironmentNonvolatile(team_a_normal, team_b, true);
  engine_->setEnvironment(env);

  auto r1 = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  auto r1_env = r1.where1().getEnv();

  // Snorlax should NOT have lost HP
  EXPECT_NEAR(r1_env.teammate(0, 0).getPercentHP(), 1.0, 0.01);

  // Snorlax Stats: Speed -1, Atk +1, Def +1
  // We can check boosts
  EXPECT_EQ(r1_env.teammate(0, 0).getBoost(FV_SPEED), -1);
  EXPECT_EQ(r1_env.teammate(0, 0).getBoost(FV_ATTACK), 1);
  EXPECT_EQ(r1_env.teammate(0, 0).getBoost(FV_DEFENSE), 1);

  // Target should NOT have Curse status
  EXPECT_FALSE(r1_env.teammate(1, 0).status().cTeammate.curse);
}

TEST_F(CurseTest, GhostCurse_FailsIfTargetAlreadyCursed) {
  // Gengar uses Curse on Snorlax twice
  auto env = EnvironmentNonvolatile(team_a_ghost, team_b, true);
  engine_->setEnvironment(env);

  // Turn 1
  auto r1 = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  // Turn 2: Gengar uses Curse again (should fail)
  auto r2 = engine_->updateState(
      r1.where1(), Action::move(0), Action::wait());

  auto r2_env = r2.where1().getEnv();

  // Gengar HP should still be 0.5 (didn't pay cost again?)
  // Actually, if move fails, does it pay cost?
  // implementation: if (tPKV.status().cTeammate.curse) { return 0; }
  // return 0 means "move failed" or "plugin didn't handle"?
  // If plugin returns 0, the engine might proceed to other plugins or default behavior?
  // But Curse is "Unknown" type and likely has no default implementation.
  // PkCUEngine::evaluateMove_script calls plugin. If plugin returns 0, it does nothing?
  // `result` starts at `isImplemented()`?
  // In `evaluateMove_script`:
  // `int result = mV.getBase().isImplemented()?1:0;`
  // Since we haven't modified `Move::isImplemented`, it likely returns false (0).
  // `CALLPLUGIN` ORs the result.
  // So if `move_curse_set` returns 0, `result` remains 0.
  // Does that mean "move failed"?
  // Usually failure is indicated by NOT modifying anything.
  // So HP shouldn't change.

  EXPECT_NEAR(r2_env.teammate(0, 0).getPercentHP(), 0.5, 0.01);
}
