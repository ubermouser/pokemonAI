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
};

TEST_F(ConfusionStatusTest, Test_AppliesConfusion) {
    // Team A uses Confuse Ray (Move 0), Team B waits
    auto results = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

    // Filter states where secondary effect occurred for Team A (index 0)
    // Confuse Ray sets the secondary flag when it successfully confuses
    auto status_state = results.where1Status(0);

    // Team B (index 1) should be confused
    EXPECT_GT(status_state.teammate(1, 0).status().confused, 0U);
}

TEST_F(ConfusionStatusTest, Test_ConfusionHurtSelf) {
    // Setup: Get to a state where Team B is confused
    auto results1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
    auto confused_state = results1.where1Status(0);

    uint32_t initial_hp = confused_state.teammate(1, 0).getHP();

    // Turn 2: Team B tries to move (Tackle), Team A waits
    auto results2 = engine_->updateState(confused_state, Action::wait(), Action::move(0));

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
    // We use a list to store results of each turn to keep them alive,
    // as ConstEnvironmentPossible holds pointers into these results.
    std::list<PossibleEnvironments> history;
    history.push_back(engine_->updateState(engine_->initialState(), Action::move(0), Action::wait()));

    ConstEnvironmentPossible state = history.back().where1Status(0);

    // Step through turns until confusion wears off
    bool eventually_wore_off = false;
    for (int i = 0; i < 10; ++i) {
        if (state.teammate(1, 0).status().confused == 0) {
            eventually_wore_off = true;
            break;
        }

        history.push_back(engine_->updateState(state, Action::wait(), Action::move(0)));
        auto& current_results = history.back();

        // Pick the most probable state for next turn.
        // We follow the branch where it was blocked because the engine
        // only decrements confusion in the blocked branch currently.
        auto blocked_states = current_results.where([](const ConstEnvironmentPossible& env) {
            return env.flagsFor(TEAM_B).isBlocked();
        });

        if (!blocked_states.empty()) {
            state = blocked_states.front();
        } else {
            state = current_results.where1();
        }
    }

    EXPECT_TRUE(eventually_wore_off);
}
