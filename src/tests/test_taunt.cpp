#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/pkai.h"
#include "pokemonai/team_volatile.h"

class TauntTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(true);

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

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
  EnvironmentNonvolatile environment_nv;
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

// TODO(@drendleman) test fails with arguments ./build/src/tests/test_taunt --gtest_shuffle --gtest_random_seed=1 --gtest_also_run_disabled_tests
TEST_F(TauntTest, DISABLED_WearsOff) {
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

  auto current_state = turn1.at(0);
  for (uint32_t i = 0; i < initial_duration; ++i) {
      // Duration should decrement each turn
      auto next_turn = engine_->updateState(current_state, Action::move(1), Action::move(1));
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
