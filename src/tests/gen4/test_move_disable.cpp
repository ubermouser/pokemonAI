#include "engine_test.hpp"

class DisableTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // Alakazam: Disable (0), Psychic (1), Taunt (2)
    auto team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("alakazam"))
            .addMove(pokedex_->move("disable"))   // 0
            .addMove(pokedex_->move("psychic"))   // 1
            .addMove(pokedex_->move("taunt"))     // 2
            .setLevel(100));

    // Blissey: Seismic Toss (0), Softboiled (1), Toxic (2)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("seismic toss")) // 0
          .addMove(pokedex_->move("softboiled"))   // 1
          .addMove(pokedex_->move("toxic"))        // 2
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    const auto initialState = engine_->initialState();

    // Chain 1: FailsIfNoMoveUsed
    // Alakazam uses Disable, Blissey uses Seismic Toss. Alakazam goes first.
    results_fails_if_no_move = engine_->updateState(initialState, Action::move(0), Action::move(0));

    // Chain 2: AppliesEffect
    // T1: Alakazam(Psychic), Blissey(Softboiled/Move 1)
    state_p_s = engine_->updateState(initialState, Action::move(1), Action::move(1));
    // T2: Alakazam(Disable), Blissey(Seismic Toss)
    // Alakazam Disables Blissey's LAST move (Softboiled).
    results_applies_effect = engine_->updateState(state_p_s.where1(), Action::move(0), Action::move(0));

    // Chain 3: FailsIfAlreadyDisabled
    // From results_applies_effect, use Disable again.
    state_after_disabled = engine_->updateState(results_applies_effect.where1(), Action::move(0), Action::move(2));

    // Chain 4: RestrictsMoves
    // Blissey is Disabled on Softboiled (Move 1).
    // Try to use Softboiled -> Invalid.
    // Try to use Seismic Toss -> Valid.

    // Chain 5: ProbabilisticDuration
    // Continue from results_applies_effect (Disabled duration 7 at start of T2 end -> 6 at start of T3).
    // T3: A(Psychic), B(Seismic Toss)
    state_prob_3 = engine_->updateState(results_applies_effect.where1(), Action::move(1), Action::move(0));
    // T4
    state_prob_4 = engine_->updateState(state_prob_3.where1(), Action::move(1), Action::move(0));
    // T5
    state_prob_5 = engine_->updateState(state_prob_4.where1(), Action::move(1), Action::move(0));

    // Filter for duration 3 (logic is same as Encore: 7->6->5->4->branch)
    // T2 end: 6
    // T3 end: 5
    // T4 end: 4
    // T5 end: 3 or 0.

    auto state5_filtered = state_prob_5.where1([](const ConstEnvironmentPossible& res) {
        return res.teammate(1, 0).status().cTeammate.disable_duration == 3;
    });

    // T6
    state_prob_6 = engine_->updateState(state5_filtered, Action::move(1), Action::move(0));
  }

  PossibleEnvironments results_fails_if_no_move;
  PossibleEnvironments state_p_s;
  PossibleEnvironments results_applies_effect;
  PossibleEnvironments state_after_disabled;
  PossibleEnvironments state_prob_3;
  PossibleEnvironments state_prob_4;
  PossibleEnvironments state_prob_5;
  PossibleEnvironments state_prob_6;
};

TEST_F(DisableTest, FailsIfNoMoveUsed) {
  auto env = results_fails_if_no_move.where1().getEnv();
  EXPECT_EQ(env.teammate(1, 0).status().cTeammate.disable_duration, 0);
}

TEST_F(DisableTest, AppliesEffect) {
  auto env = results_applies_effect.where1().getEnv();
  auto teamStatus = env.teammate(1, 0).status();

  // Blissey used Softboiled (Move 1) in T1.
  // Disable should disable Move 1.
  EXPECT_GT(teamStatus.cTeammate.disable_duration, 0);
  EXPECT_EQ(teamStatus.cTeammate.disable_action, 1);
}

TEST_F(DisableTest, RestrictsMoves) {
  auto state = results_applies_effect.where1();

  // Blissey Disabled on Move 1 (Softboiled).

  // Try Softboiled (Move 1) - Should be INVALID
  EXPECT_FALSE(engine_->isValidAction(state, Action::move(1), TEAM_B));

  // Try Seismic Toss (Move 0) - Should be VALID
  EXPECT_TRUE(engine_->isValidAction(state, Action::move(0), TEAM_B));

  // Try Toxic (Move 2) - Should be VALID
  EXPECT_TRUE(engine_->isValidAction(state, Action::move(2), TEAM_B));
}

TEST_F(DisableTest, FailsIfAlreadyDisabled) {
  auto state2 = results_applies_effect.where1();
  auto durationAfterTurn2 = state2.teammate(1, 0).status().cTeammate.disable_duration;

  // Turn 3: Alakazam uses Disable again.
  auto state3 = state_after_disabled.where1();

  // Should decrement (end of turn) but NOT reset (set fails).
  EXPECT_EQ(state3.teammate(1, 0).status().cTeammate.disable_duration, durationAfterTurn2 - 1);
}

TEST_F(DisableTest, ProbabilisticDuration) {
  // Similar logic to Encore logic

  // T2 end: 6 (results_applies_effect)
  EXPECT_EQ(results_applies_effect.where1().teammate(1, 0).status().cTeammate.disable_duration, 6);

  // T3 end: 5
  EXPECT_EQ(state_prob_3.where1().teammate(1, 0).status().cTeammate.disable_duration, 5);

  // T4 end: 4
  EXPECT_EQ(state_prob_4.where1().teammate(1, 0).status().cTeammate.disable_duration, 4);

  // T5 end: 3 (75%) or 0 (25%)
  EXPECT_GE(state_prob_5.size(), 2);

  // T6 end from state5_filtered (dur 3): 2 (66%) or 0 (33%)
  EXPECT_GE(state_prob_6.size(), 2);

  bool foundEnded = false;
  bool foundContinued = false;

  for (size_t i = 0; i < state_prob_6.size(); ++i) {
    auto res = state_prob_6.at(i);
    if (res.teammate(1, 0).status().cTeammate.disable_duration == 0) {
      foundEnded = true;
    } else {
      foundContinued = true;
      EXPECT_EQ(res.teammate(1, 0).status().cTeammate.disable_duration, 2);
    }
  }

  EXPECT_TRUE(foundEnded);
  EXPECT_TRUE(foundContinued);
}
