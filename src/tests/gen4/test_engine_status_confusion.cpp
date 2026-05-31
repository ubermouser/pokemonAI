#include "engine_test.hpp"
#include <list>

class ConfusionStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Smeargle with Confuse Ray
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("confuse ray"))
          .setLevel(100));

    // Team B: Smeargle with Tackle
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupConfusionApplied() {
    return engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupConfusionTurn2() {
    auto results = setupConfusionApplied();
    return engine_->updateState(
        results.where1Status(0).getEnv(),
        Action::wait(),
        Action::move(0));
  }

  ConstEnvironmentPossible advanceConfusion(const ConstEnvironmentPossible& state, std::list<PossibleEnvironments>& history) {
    history.push_back(engine_->updateState(state.getEnv(), Action::wait(), Action::move(0)));
    auto& results = history.back();
    auto blocked_states = results.where([](const ConstEnvironmentPossible& env) {
        return env.flagsFor(TEAM_B).isBlocked();
    });
    return !blocked_states.empty() ? blocked_states.front() : results.where1();
  }
};

TEST_F(ConfusionStatusTest, Test_AppliesConfusion) {
    // Team A uses Confuse Ray (Move 0), Team B waits
    auto results = setupConfusionApplied();

    // Filter states where secondary effect occurred for Team A (index 0)
    // Confuse Ray sets the secondary flag when it successfully confuses
    auto status_state = results.where1Status(0);

    // Team B (index 1) should be confused
    EXPECT_GT(status_state.teammate(1, 0).status().confused, 0U);
}

TEST_F(ConfusionStatusTest, Test_ConfusionHurtSelf) {
    // Setup: Get to a state where Team B is confused
    auto confused_results = setupConfusionApplied();
    auto confused_state = confused_results.where1Status(0);
    uint32_t initial_hp = confused_state.teammate(1, 0).getHP();

    // Turn 2: Team B tries to move (Tackle), Team A waits
    auto results2 = setupConfusionTurn2();

    // There should be a 50% total chance to be blocked and take damage
    auto blocked_states = results2.where([](const ConstEnvironmentPossible& env) {
        return env.flagsFor(TEAM_B).isBlocked();
    });

    ASSERT_FALSE(blocked_states.empty());

    double total_blocked_prob = 0;
    for (const auto& state : blocked_states) {
        // In blocked states, Team B should have taken 40 damage
        EXPECT_EQ(state.teammate(1, 0).getHP(), initial_hp - 40);
        total_blocked_prob += state.getProbability().to_double();
    }
    EXPECT_NEAR(total_blocked_prob, 0.5, 0.01);

    // There should also be a 50% total chance to NOT be blocked
    auto not_blocked_states = results2.where([](const ConstEnvironmentPossible& env) {
        return !env.flagsFor(TEAM_B).isBlocked();
    });

    ASSERT_FALSE(not_blocked_states.empty());
    double total_not_blocked_prob = 0;
    for (const auto& state : not_blocked_states) {
        total_not_blocked_prob += state.getProbability().to_double();
    }
    EXPECT_NEAR(total_not_blocked_prob, 0.5, 0.01);
}

TEST_F(ConfusionStatusTest, Test_ConfusionWearsOff) {
    // Setup: Get to a state where Team B is confused
    std::list<PossibleEnvironments> history;
    history.push_back(setupConfusionApplied());

    ConstEnvironmentPossible state = history.back().where1Status(0);

    // Step through turns until confusion wears off
    bool eventually_wore_off = false;
    for (int i = 0; i < 10; ++i) {
        if (state.teammate(1, 0).status().confused == 0) {
            eventually_wore_off = true;
            break;
        }

        state = advanceConfusion(state, history);
    }

    EXPECT_TRUE(eventually_wore_off);
}

TEST_F(ConfusionStatusTest, Test_StateTransitionPrinterConfusion) {
    auto results = setupConfusionApplied();
    auto confused_state = results.where1Status(0);
    std::string output = StateTransitionPrinter::printString(
        engine_->initialState(),
        confused_state,
        /*withStyle=*/false);
    EXPECT_TRUE(output.find("became confused!") != std::string::npos)
        << "Expected 'became confused!' in printer output: " << output;
}
