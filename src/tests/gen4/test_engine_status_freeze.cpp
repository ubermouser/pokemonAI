#include "engine_test.hpp"

class FreezeStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
  }

  void SetupFrozenPokemon(size_t teamIndex, size_t teammateIndex) {
      auto team_a = TeamNonVolatile()
          .addPokemon(PokemonNonVolatile()
              .setBase(pokedex_->pokemon("smeargle"))
              .addMove(pokedex_->move("ice beam"))
              .addMove(pokedex_->move("flame wheel"))
              .setLevel(100));

      auto team_b = TeamNonVolatile()
          .addPokemon(PokemonNonVolatile()
              .setBase(pokedex_->pokemon("smeargle"))
              .addMove(pokedex_->move("tackle"))
              .addMove(pokedex_->move("flamethrower"))
              .setLevel(100));

      engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

      auto initialEnv = engine_->initialState();
      mutableData = initialEnv.data();

      // Set Freeze Ailment
      mutableData.teams[teamIndex].teammates[teammateIndex].status_nonvolatile = AIL_NV_FREEZE;

      // Important: Use initialEnv.nv() so pointers match
      frozen_env = std::make_shared<EnvironmentVolatile>(initialEnv.nv(), mutableData);
  }

  EnvironmentVolatileData mutableData;
  std::shared_ptr<EnvironmentVolatile> frozen_env;
};

TEST_F(FreezeStatusTest, Test_AppliesFreeze) {
    // Team A: Smeargle with Ice Beam
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("ice beam"))
          .setLevel(100));

    // Team B: Smeargle
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

    // Smeargle uses Ice Beam (Move 0) against Smeargle
    auto results = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

    // Filter states where secondary effect occurred for Team A (index 0)
    auto status_states = results.whereStatus(0);

    // Ice Beam has 10% secondary chance, so we should expect some states.
    ASSERT_FALSE(status_states.empty()) << "Ice Beam secondary effect did not trigger in any environment.";

    // Target (Team B / index 1) should be frozen in these states
    EXPECT_EQ(status_states.front().teammate(1, 0).getStatusAilment(), AIL_NV_FREEZE);
}

TEST_F(FreezeStatusTest, Test_FrozenPokemonCannotMove) {
    SetupFrozenPokemon(0, 0); // Team A Smeargle is frozen

    // Smeargle tries to use Ice Beam, Team B waits
    auto results = engine_->updateState(*frozen_env, Action::move(0), Action::wait());

    // In most states, Smeargle should be blocked
    auto blocked_states = results.where([](const ConstEnvironmentPossible& env) {
        return env.wasBlocked(0);
    });

    EXPECT_FALSE(blocked_states.empty());

    // In blocked states, Team B Smeargle should have full HP
    for (const auto& state : blocked_states) {
      EXPECT_EQ(state.teammate(1, 0).getPercentHP(), 1.);
    }
}

TEST_F(FreezeStatusTest, Test_ThawProbabilistic) {
    SetupFrozenPokemon(0, 0); // Team A Smeargle is frozen

    // Smeargle tries to move
    auto results = engine_->updateState(*frozen_env, Action::move(0), Action::wait());

    // There should be a 20% chance to thaw?
    auto thaw_states = results.where([](const ConstEnvironmentPossible& env) {
        return env.teammate(0, 0).getStatusAilment() == AIL_NV_NONE;
    });
    bool found_thaw = !thaw_states.empty();

    EXPECT_TRUE(found_thaw) << "Pokemon should have a chance to thaw naturally";
}

TEST_F(FreezeStatusTest, Test_ThawByUsingFireMove) {
    SetupFrozenPokemon(0, 0); // Team A Smeargle is frozen

    // Smeargle uses Flame Wheel (Move 1), which is Fire-type and thaws the user
    auto results = engine_->updateState(*frozen_env, Action::move(1), Action::wait());

    // It should thaw and move
    auto thaw_states = results.where([](const ConstEnvironmentPossible& env) {
        return env.teammate(0, 0).getStatusAilment() == AIL_NV_NONE;
    });

    EXPECT_FALSE(thaw_states.empty()) << "Flame Wheel should thaw the user";

    // In thaw states, it should have hit Team B Smeargle
    for (const auto& state : thaw_states) {
        EXPECT_LT(state.teammate(1, 0).getHP(), state.teammate(1, 0).nv().getMaxHP());
    }
}

TEST_F(FreezeStatusTest, Test_ThawByBeingHitByFireMove) {
    SetupFrozenPokemon(1, 0); // Team B Smeargle is frozen

    // Team A Smeargle uses Flame Wheel (Move 1) on Team B Smeargle
    auto results = engine_->updateState(*frozen_env, Action::move(1), Action::wait());

    // Team B Smeargle should thaw after being hit
    auto thaw_states = results.where([](const ConstEnvironmentPossible& env) {
        return env.teammate(1, 0).getStatusAilment() == AIL_NV_NONE;
    });

    EXPECT_FALSE(thaw_states.empty()) << "Pokemon should thaw after being hit by a Fire-type move";
}
