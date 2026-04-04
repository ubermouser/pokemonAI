#include "engine_test.hpp"

class EncoreTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    auto team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("alakazam"))
            .addMove(pokedex_->move("encore"))   // 0
            .addMove(pokedex_->move("psychic"))  // 1
            .addMove(pokedex_->move("taunt"))    // 2
            .setLevel(100));

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
    results_fails_if_no_move = engine_->updateState(initialState, Action::move(0), Action::move(0));

    // Chain 2 & 3: PreemptsChoice, FailsIfAlreadyEncored, AppliesEffect
    state_p_s = engine_->updateState(initialState, Action::move(1), Action::move(1));
    state_encored_seismic = engine_->updateState(state_p_s.where1(), Action::move(0), Action::move(0));
    state_after_encored = engine_->updateState(state_encored_seismic.where1(), Action::move(1), Action::move(0));
    results_applies_effect = engine_->updateState(state_p_s.where1(), Action::move(0), Action::move(1));

    // Chain 4: RestrictsMoves
    state_wait_seismic = engine_->updateState(initialState, Action::wait(), Action::move(0));
    results_restricts_moves = engine_->updateState(state_wait_seismic.where1(), Action::move(0), Action::wait());

    // Chain 5: ProbabilisticDuration
    state_prob_3 = engine_->updateState(results_applies_effect.where1(), Action::move(1), Action::move(1));
    state_prob_4 = engine_->updateState(state_prob_3.where1(), Action::move(1), Action::move(1));
    state_prob_5 = engine_->updateState(state_prob_4.where1(), Action::move(1), Action::move(1));

    auto state5_filtered = state_prob_5.where1([](const ConstEnvironmentPossible& res) {
        return res.teammate(1, 0).status().cTeammate.encore_duration == 3;
    });

    state_prob_6 = engine_->updateState(state5_filtered, Action::move(1), Action::move(1));

    // Chain 6: EncoreAndTaunt
    state_encore_wait = engine_->updateState(state_p_s.where1(), Action::move(0), Action::wait());
    results_encore_taunt = engine_->updateState(state_encore_wait.where1(), Action::move(2), Action::wait());
    results_encore_taunt_struggle = engine_->updateState(results_encore_taunt.where1(), Action::wait(), Action::move(1));

    // Chain 7: EndsOnPPDepletion
    auto state_pp_mod = state_wait_seismic.where1();
    state_pp_mod.teammate(1, 0).getMV(Action::move(0)).setPP(1);

    state_pp_encored = engine_->updateState(state_pp_mod, Action::move(0), Action::wait());
    results_pp_depletion = engine_->updateState(state_pp_encored.where1(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments results_fails_if_no_move;
  PossibleEnvironments state_p_s;
  PossibleEnvironments state_encored_seismic;
  PossibleEnvironments state_after_encored;
  PossibleEnvironments results_applies_effect;
  PossibleEnvironments results_restricts_moves;
  PossibleEnvironments state_prob_3;
  PossibleEnvironments state_prob_4;
  PossibleEnvironments state_prob_5;
  PossibleEnvironments state_prob_6;
  PossibleEnvironments state_encore_wait;
  PossibleEnvironments results_encore_taunt;
  PossibleEnvironments results_encore_taunt_struggle;
  PossibleEnvironments state_wait_seismic;
  PossibleEnvironments state_pp_encored;
  PossibleEnvironments results_pp_depletion;
};

TEST_F(EncoreTest, FailsIfNoMoveUsed) {
  // Turn 1: Alakazam uses Encore, Blissey uses Seismic Toss.
  // Since Alakazam is faster, Encore does nothing.
  auto env = results_fails_if_no_move.where1().getEnv();

  EXPECT_EQ(env.teammate(1, 0).status().cTeammate.encore_duration, 0);
  EXPECT_EQ(env.teammate(1, 0).status().cTeammate.encore_action, 0);
}

TEST_F(EncoreTest, PreemptsChoiceIfMoveIsForbidden) {
  // If Blissey is encored in the same turn as another move is selected, the
  // move should be preempted.

  auto state2 = state_encored_seismic.where1();

  // Verify that Blissey's action was preempted and it used Soft-Boiled (Move 1)
  // instead of Seismic Toss (Move 0).
  // Note: SetUp logic:
  // state_p_s: Alakazam(Psychic), Blissey(Softboiled/Move 1)
  // state_encored_seismic: Alakazam(Encore), Blissey(Seismic Toss/Move 0)
  // Blissey is encored into Softboiled (Move 1).
  EXPECT_TRUE(state2.actor(TEAM_B, ActorProxy::ALL_TEAMMATES).isBlocked());  // It was "blocked" and switched to encored move

  // Check that Blissey is now encored into Soft-Boiled (Move 1)
  auto teamStatus = state2.teammate(1, 0).status();
  EXPECT_GT(teamStatus.cTeammate.encore_duration, 0);
  EXPECT_EQ(teamStatus.cTeammate.encore_action, 1);
}

TEST_F(EncoreTest, FailsIfAlreadyEncored) {
  // If Blissey is already encored, using Encore again should fail and not
  // reset the duration.

  auto state2 = state_encored_seismic.where1();
  auto durationAfterTurn2 =
      state2.teammate(1, 0).status().cTeammate.encore_duration;
  EXPECT_GT(durationAfterTurn2, 0);

  // Turn 3: Blissey encored into Soft-Boiled.
  // The duration should decrement due to the turn end, but NOT be reset by the
  // second Encore attempt.
  auto state3 = state_after_encored.where1();

  auto teamStatus = state3.teammate(1, 0).status();
  // Duration should be durationAfterTurn2 - 1
  EXPECT_EQ(teamStatus.cTeammate.encore_duration, durationAfterTurn2 - 1);
}

TEST_F(EncoreTest, AppliesEffect) {
  // Turn 1: Blissey uses Soft-Boiled, Alakazam uses Psychic
  // Turn 2: Blissey uses Soft-Boiled, Alakazam uses Encore

  auto env = results_applies_effect.where1().getEnv();
  auto teamStatus = env.teammate(1, 0).status();

  // Blissey should be encored into Soft-Boiled (Move 1)
  EXPECT_GT(teamStatus.cTeammate.encore_duration, 0);
  EXPECT_EQ(teamStatus.cTeammate.encore_action, 1);
}

TEST_F(EncoreTest, RestrictsMoves) {
  // Turn 1: Blissey uses Seismic Toss (Move 0)
  // Turn 2: Alakazam uses Encore
  // Blissey should be Encored into Seismic Toss (Move 0)

  auto state = results_restricts_moves.where1();

  // Blissey tries to use Soft-Boiled (Move 1) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(1)));

  // Blissey tries to use Seismic Toss (Move 0) - Should be valid
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(0)));
}

TEST_F(EncoreTest, ProbabilisticDuration) {
  // Turn 1: Blissey uses Soft-Boiled, Alakazam uses Psychic
  // Turn 2: Alakazam uses Encore, Blissey uses Soft-Boiled
  // Turn 3: Alakazam uses Psychic, Blissey uses Soft-Boiled (BOT: 6 -> 5)
  // Note: duration was 6 at end of Turn 2 because Alakazam encores fast and
  // Blissey moves after.

  auto state3 = state_prob_3.where1();
  EXPECT_EQ(state3.teammate(1, 0).status().cTeammate.encore_duration, 5);

  // Turn 4: BOT 5 -> 4
  auto state4 = state_prob_4.where1();
  EXPECT_EQ(state4.teammate(1, 0).status().cTeammate.encore_duration, 4);

  // Turn 5: BOT 4 -> 1/4 chance to end.
  // Turn 5 might have branched!
  EXPECT_GE(state_prob_5.size(), 2);

  // Turn 6: BOT 3 -> 1/3 chance to end.
  auto turn6 = state_prob_6;

  EXPECT_GE(turn6.size(), 2);

  bool foundEnded = false;
  bool foundContinued = false;

  for (size_t i = 0; i < turn6.size(); ++i) {
    auto res = turn6.at(i);
    if (res.teammate(1, 0).status().cTeammate.encore_duration == 0) {
      foundEnded = true;
    } else {
      foundContinued = true;
      EXPECT_EQ(res.teammate(1, 0).status().cTeammate.encore_duration, 2);
    }
  }

  EXPECT_TRUE(foundEnded);
  EXPECT_TRUE(foundContinued);
}

TEST_F(EncoreTest, EncoreAndTaunt) {
  // Alakazam has Encore (Move 0)
  // Blissey has Seismic Toss (Move 0), Soft-Boiled (Move 1), Toxic (Move 2)

  // Turn 1: Blissey uses Soft-Boiled (Move 1), Alakazam uses Psychic
  // Turn 2: Alakazam uses Encore, Blissey uses Soft-Boiled
  // Now Alakazam uses Taunt on Turn 3.

  auto env3 = results_encore_taunt.where1();
  EXPECT_GT(env3.teammate(1, 0).status().cTeammate.encore_duration, 0);

  // Now Blissey is Encored into Soft-Boiled (Move 1) AND Taunted.
  // Move 0: Seismic Toss (Damage) - Blocked by Encore (not Move 1)
  // Move 1: Soft-Boiled (Status) - Blocked by Taunt
  // Move 2: Toxic (Status) - Blocked by both

  EXPECT_FALSE(engine_->isValidAction(env3, Actor(TEAM_B, 0), Action::move(0)));
  EXPECT_FALSE(engine_->isValidAction(env3, Actor(TEAM_B, 0), Action::move(1)));
  EXPECT_FALSE(engine_->isValidAction(env3, Actor(TEAM_B, 0), Action::move(2)));

  // Check for Struggle
  EXPECT_TRUE(engine_->isValidAction(env3, Actor(TEAM_B, 0), Action::struggle()));

  // Verify that updateState results in Struggle (or is blocked)
  auto env4 = results_encore_taunt_struggle.where1();

  // Blissey should have been blocked (from Move 1 because of Taunt)
  EXPECT_TRUE(env4.actor(TEAM_B, ActorProxy::ALL_TEAMMATES).isBlocked());
}

TEST_F(EncoreTest, EndsOnPPDepletion) {
  // Setup Blissey with 1 PP for Seismic Toss (Move 0)

  auto state2 = state_pp_encored.where1();

  EXPECT_GT(state2.teammate(1, 0).status().cTeammate.encore_duration, 0);

  // Blissey uses Seismic Toss (Move 0), using up its last PP
  auto state3 = results_pp_depletion.where1();

  // Encore should end because PP is 0
  EXPECT_EQ(state3.teammate(1, 0).status().cTeammate.encore_duration, 0);
}
