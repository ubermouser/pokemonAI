#include "engine_test.hpp"

class FreezeStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("ice beam"))
          .addMove(pokedex_->move("flame wheel"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("ice beam"))
          .addMove(pokedex_->move("flamethrower"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));
  }

  PossibleEnvironments setupFreezeTeamB() {
    return engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupFreezeTeamA() {
    return engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments setupFrozenCannotMove() {
    auto results = setupFreezeTeamA();
    auto frozen_state = results.whereStatus(1).front();
    return engine_->updateState(frozen_state.getEnv(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupThawByUsingFireMove() {
    auto results = setupFreezeTeamA();
    auto frozen_state = results.whereStatus(1).front();
    return engine_->updateState(frozen_state.getEnv(), Action::move(1), Action::wait());
  }

  PossibleEnvironments setupThawByBeingHitByFireMove() {
    auto results = setupFreezeTeamB();
    auto frozen_state = results.whereStatus(0).front();
    return engine_->updateState(frozen_state.getEnv(), Action::move(1), Action::wait());
  }
};

TEST_F(FreezeStatusTest, Test_AppliesFreeze) {
    auto results = setupFreezeTeamB();

    // Filter states where secondary effect occurred for Team A (index 0)
    auto status_states = results.whereStatus(0);

    // Ice Beam has 10% secondary chance, so we should expect some states.
    ASSERT_FALSE(status_states.empty()) << "Ice Beam secondary effect did not trigger in any environment.";

    // Target (Team B / index 1) should be frozen in these states
    EXPECT_EQ(status_states.front().teammate(1, 0).getStatusAilment(), AIL_NV_FREEZE);
}

TEST_F(FreezeStatusTest, Test_FrozenPokemonCannotMove) {
    auto results = setupFrozenCannotMove();

    // In most states, Smeargle should be blocked
    auto blocked_states = results.where([](const ConstEnvironmentPossible& env) {
        return env.flagsFor(TEAM_A).isBlocked();
    });

    EXPECT_FALSE(blocked_states.empty());

    // In blocked states, Team B Smeargle should have full HP
    for (const auto& state : blocked_states) {
      EXPECT_EQ(state.teammate(1, 0).getPercentHP(), 1.);
    }
}

TEST_F(FreezeStatusTest, Test_ThawProbabilistic) {
    auto results = setupFrozenCannotMove();

    // There should be a 20% chance to thaw
    auto thaw_states = results.where([](const ConstEnvironmentPossible& env) {
        return env.teammate(0, 0).getStatusAilment() == AIL_NV_NONE;
    });
    bool found_thaw = !thaw_states.empty();

    EXPECT_TRUE(found_thaw) << "Pokemon should have a chance to thaw naturally";
}

TEST_F(FreezeStatusTest, Test_ThawByUsingFireMove) {
    auto results = setupThawByUsingFireMove();

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
    auto results = setupThawByBeingHitByFireMove();

    // Team B Smeargle should thaw after being hit
    auto thaw_states = results.where([](const ConstEnvironmentPossible& env) {
        return env.teammate(1, 0).getStatusAilment() == AIL_NV_NONE;
    });

    EXPECT_FALSE(thaw_states.empty()) << "Pokemon should thaw after being hit by a Fire-type move";
}

TEST_F(FreezeStatusTest, Test_StateTransitionPrinterFreeze) {
    auto results = setupFreezeTeamB();
    auto status_states = results.whereStatus(0);
    ASSERT_FALSE(status_states.empty());

    std::string output = StateTransitionPrinter::printString(
        engine_->initialState(),
        status_states.front(),
        /*withStyle=*/false);
    EXPECT_TRUE(output.find("was frozen solid!") != std::string::npos)
        << "Expected 'was frozen solid!' in printer output: " << output;
}
