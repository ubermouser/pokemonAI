#include "engine_test.hpp"


class TauntTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("steelix"))
          .addMove(pokedex_->move("taunt"))
          .addMove(pokedex_->move("strength")) // damage
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu"))); // backup

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("shuckle"))
          .addMove(pokedex_->move("toxic")) // status
          .addMove(pokedex_->move("strength")) // damage
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(TauntTest, AppliesEffect) {
  // Steelix uses Taunt on Shuckle
  auto taunt_result = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto env = taunt_result.at(0).getEnv();

  // Shuckle should have taunt duration
  EXPECT_GT(env.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);
  // Steelix should not
  EXPECT_EQ(env.getTeam(0).teammate(0).status().cTeammate.taunt_duration, 0);
}

TEST_F(TauntTest, PreventsStatusMoves) {
  // Steelix uses Taunt on Shuckle
  auto taunt_result = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto env = taunt_result.at(0).getEnv();

  // Shuckle tries to use Toxic (Move 0, Status) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(taunt_result.at(0), Action::move(0), TEAM_B));

  // Shuckle tries to use Constrict (Move 1, Physical) - Should be valid
  EXPECT_TRUE(engine_->isValidAction(taunt_result.at(0), Action::move(1), TEAM_B));
}

TEST_F(TauntTest, WearsOff) {
  // Turn 1: Steelix uses Taunt. Shuckle is taunted (duration 3-5).
  // Note: PkCU branches state. We pick the first environment and follow it.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  turn1.printStates();
  // Verify we have multiple outcomes (3, 4, 5 turns)
  // We expect at least 3 states due to triplicateState, possibly more if other RNG happened (but unlikely here)
  EXPECT_GE(turn1.size(), 3);

  // Pick one state to follow.
  auto initial_env = turn1.at(0);
  auto initial_duration = initial_env.getEnv().getTeam(1).teammate(0).status().cTeammate.taunt_duration;

  EXPECT_GE(initial_duration, 3);
  EXPECT_LE(initial_duration, 5);

  auto next_turn = turn1;
  auto current_state = turn1.at(0);
  for (uint32_t i = 0; i < initial_duration; ++i) {
      // Duration should decrement each turn
      next_turn = engine_->updateState(current_state, Action::move(1), Action::move(1));
      current_state = next_turn.at(0);
      next_turn.printStates();

      auto current_duration = current_state.getEnv().getTeam(1).teammate(0).status().cTeammate.taunt_duration;
      EXPECT_EQ(current_duration, initial_duration - 1 - i);
  }

  // After duration expires, duration should be 0
  EXPECT_EQ(current_state.getEnv().getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);

  // Now Shuckle can use Toxic again.
  EXPECT_TRUE(engine_->isValidAction(current_state, Action::move(0), TEAM_B));
}


TEST_F(TauntTest, PreemptsStatusMoveSameTurn) {
  // Scenario:
  // Steelix (Fast, uses Taunt) vs Shuckle (Slow, uses Toxic)
  // Steelix moves first. Taunt should apply.
  // Shuckle attempts Toxic. It should fail because it is taunted in the same turn.
  engine_->setEnvironment(environment_nv);

  // Action: P0 uses Taunt (Move 0), P1 uses Toxic (Move 0)
  auto result = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));

  auto final_env = result.at(0).getEnv();

  // 1. Shuckle should be taunted
  EXPECT_GT(final_env.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);

  // 2. Steelix should NOT be poisoned (Toxic should have failed)
  EXPECT_EQ(final_env.getTeam(0).teammate(0).getStatusAilment(), AIL_NV_NONE);
}
