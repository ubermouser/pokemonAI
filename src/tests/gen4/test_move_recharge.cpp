#include "engine_test.hpp"

class RechargeTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // Team A: Porygon-Z and Alakazam
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("porygon-z"))
          .addMove(pokedex_->move("hyper beam"))
          .addMove(pokedex_->move("giga impact"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("alakazam"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    // Team B: Blissey (High HP to survive)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("softboiled"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(RechargeTest, RechargesAfterHit) {
  // Turn 1: Porygon-Z uses Hyper Beam.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto state1 = turn1.where1Hit(0);

  // Verify recharge status is set (lockIn_action = 7, duration = 1 after end-of-turn decrement)
  EXPECT_EQ(state1.teammate(0, 0).status().cTeammate.lockIn_action, 7U);
  EXPECT_EQ(state1.teammate(0, 0).status().cTeammate.lockIn_duration, 1U);

  // Turn 2: Porygon-Z tries to use Tackle, but should be blocked.
  // Note: we must use a valid action or the engine will throw if allowInvalidMoves is false.
  // Since isValidAction should return false for Tackle, we might need to force it or check validation.

  auto validation = engine_->isValidAction(state1, Action::move(2), 0);
  EXPECT_FALSE(validation);

  // If we force it (by setting allowInvalidMoves), the engine should still block it.
  engine_->setAllowInvalidMoves(true);
  auto turn2 = engine_->updateState(state1, Action::move(2), Action::move(0));
  auto state2 = turn2.where1();

  // Verify that the pokemon was blocked and did nothing
  EXPECT_TRUE(state2.wasBlocked(0));

  // Verify recharge status is cleared after turn 2
  EXPECT_EQ(state2.teammate(0, 0).status().cTeammate.lockIn_action, 0U);
  EXPECT_EQ(state2.teammate(0, 0).status().cTeammate.lockIn_duration, 0U);
}

TEST_F(RechargeTest, NoRechargeAfterMiss) {
  // Hyper Beam has 90% accuracy.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));

  auto missStates = turn1.whereMiss(0);
  ASSERT_FALSE(missStates.empty());

  auto state1 = missStates[0];
  EXPECT_EQ(state1.teammate(0, 0).status().cTeammate.lockIn_action, 0U);
  EXPECT_EQ(state1.teammate(0, 0).status().cTeammate.lockIn_duration, 0U);
}

TEST_F(RechargeTest, CannotSwitchDuringRecharge) {
  // Turn 1: Hyper Beam hits
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto state1 = turn1.where1Hit(0);

  // Turn 2: Attempt to switch to Alakazam (Action::swap(1))
  auto validation = engine_->isValidAction(state1, Action::swap(1), 0);
  EXPECT_FALSE(validation);
}

TEST_F(RechargeTest, AllRechargeMovesWork) {
  std::vector<std::string> moves = {
    "blast burn", "frenzy plant", "giga impact", "hydro cannon", "hyper beam", "rock wrecker"
  };

  for (const auto& moveName : moves) {
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move(moveName))
          .setLevel(100));

    EnvironmentNonvolatile env_nv(team_a, environment_nv.getTeam(1), true);
    engine_->setEnvironment(env_nv);

    auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    auto state1 = turn1.where1Hit(0);

    EXPECT_EQ(state1.teammate(0, 0).status().cTeammate.lockIn_action, 7U) << "Move " << moveName << " failed to set recharge action";
    EXPECT_EQ(state1.teammate(0, 0).status().cTeammate.lockIn_duration, 1U) << "Move " << moveName << " failed to set recharge duration";
  }
}
