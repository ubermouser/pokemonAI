#include "engine_test.hpp"

class EncoreTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    auto team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("alakazam"))
            .addMove(pokedex_->move("encore"))
            .addMove(pokedex_->move("psychic"))
            .addMove(pokedex_->move("taunt"))
            .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("seismic toss"))
          .addMove(pokedex_->move("softboiled"))
          .addMove(pokedex_->move("toxic"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(EncoreTest, FailsIfNoMoveUsed) {
  // Turn 1: Alakazam uses Encore, Blissey uses Seismic Toss.
  // Since Alakazam is faster, Encore does nothing.
  auto results = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));
  auto env = results.where1().getEnv();

  EXPECT_EQ(env.teammate(1, 0).status().cTeammate.encore_duration, 0);
  EXPECT_EQ(env.teammate(1, 0).status().cTeammate.encore_action, 0);
}

TEST_F(EncoreTest, PreemptsChoiceIfMoveIsForbidden) {
  // If Blissey is encored in the same turn as another move is selected, the
  // move should be preempted.

  // Turn 1: Blissey uses Soft-Boiled (Move 1). Alakazam uses Psychic (faster).
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::move(1));
  auto state1 = turn1.where1();

  // Turn 2: Blissey selects Seismic Toss (Move 0). Alakazam uses Encore
  // (faster).
  auto turn2 = engine_->updateState(state1, Action::move(0), Action::move(0));
  auto state2 = turn2.where1();

  // Verify that Blissey's action was preempted and it used Soft-Boiled (Move 1)
  // instead of Seismic Toss (Move 0).
  EXPECT_TRUE(state2.wasBlocked(
      TEAM_B));  // It was "blocked" and switched to encored move

  // Check that Blissey is now encored into Soft-Boiled (Move 1)
  auto teamStatus = state2.teammate(1, 0).status();
  EXPECT_GT(teamStatus.cTeammate.encore_duration, 0);
  EXPECT_EQ(teamStatus.cTeammate.encore_action, 1);
}

TEST_F(EncoreTest, FailsIfAlreadyEncored) {
  // If Blissey is already encored, using Encore again should fail and not
  // reset the duration.

  // Turn 1: Blissey uses Soft-Boiled (Move 1). Alakazam uses Psychic (faster).
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::move(1));
  auto state1 = turn1.where1();

  // Turn 2: Blissey selects Seismic Toss (Move 0). Alakazam uses Encore
  // (faster).
  auto turn2 = engine_->updateState(state1, Action::move(0), Action::move(0));
  auto state2 = turn2.where1();
  auto durationAfterTurn2 =
      state2.teammate(1, 0).status().cTeammate.encore_duration;
  EXPECT_GT(durationAfterTurn2, 0);

  // Turn 3: Blissey encored into Soft-Boiled. Alakazam uses Encore again.
  // The duration should decrement due to the turn end, but NOT be reset by the
  // second Encore attempt.
  auto turn3 = engine_->updateState(state2, Action::move(1), Action::move(0));
  auto state3 = turn3.where1();

  auto teamStatus = state3.teammate(1, 0).status();
  // Duration should be durationAfterTurn2 - 1
  EXPECT_EQ(teamStatus.cTeammate.encore_duration, durationAfterTurn2 - 1);
}

TEST_F(EncoreTest, AppliesEffect) {
  // Turn 1: Blissey uses Soft-Boiled, Alakazam uses Psychic
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::move(1));

  // Turn 2: Blissey uses Soft-Boiled, Alakazam uses Encore
  auto turn2 =
      engine_->updateState(turn1.where1(), Action::move(0), Action::move(1));
  auto env = turn2.where1().getEnv();
  auto teamStatus = env.teammate(1, 0).status();

  // Blissey should be encored into Soft-Boiled (Move 1)
  EXPECT_GT(teamStatus.cTeammate.encore_duration, 0);
  EXPECT_EQ(teamStatus.cTeammate.encore_action, 1);
}

TEST_F(EncoreTest, RestrictsMoves) {
  // Turn 1: Blissey uses Soft-Boiled
  auto turn1 = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  // Turn 2: Alakazam uses Encore
  auto turn2 = engine_->updateState(turn1.where1(), Action::move(0), Action::wait());
  auto state = turn2.where1();

  // Blissey tries to use Seismic Toss (Move 1) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(state, Action::move(1), TEAM_B));

  // Blissey tries to use Soft-Boiled (Move 0) - Should be valid
  EXPECT_TRUE(engine_->isValidAction(state, Action::move(0), TEAM_B));
}

TEST_F(EncoreTest, ProbabilisticDuration) {
  // Turn 1: Blissey uses Soft-Boiled, Alakazam uses Psychic
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::move(1));

  // Turn 2: Alakazam uses Encore, Blissey uses Soft-Boiled
  auto turn2 =
      engine_->updateState(turn1.where1(), Action::move(0), Action::move(1));
  auto state = turn2.where1();

  // Turn 3: Alakazam uses Psychic, Blissey uses Soft-Boiled (BOT: 6 -> 5)
  // Note: duration was 6 at end of Turn 2 because Alakazam encores fast and
  // Blissey moves after.
  auto turn3 = engine_->updateState(state, Action::move(1), Action::move(1));
  auto state3 = turn3.where1();
  EXPECT_EQ(state3.teammate(1, 0).status().cTeammate.encore_duration, 5);

  // Turn 4: BOT 5 -> 4
  auto turn4 = engine_->updateState(state3, Action::move(1), Action::move(1));
  auto state4 = turn4.where1();
  EXPECT_EQ(state4.teammate(1, 0).status().cTeammate.encore_duration, 4);

  // Turn 5: BOT 4 -> 1/4 chance to end.
  auto turn5 = engine_->updateState(state4, Action::move(1), Action::move(1));

  // Turn 5 might have branched!
  // Wait, if it's 4, it branches.
  EXPECT_GE(turn5.size(), 2);

  // Let's find one where it continued to 3.
  auto state5 = turn5.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(1, 0).status().cTeammate.encore_duration == 3;
  });

  // Turn 6: BOT 3 -> 1/3 chance to end.
  auto turn6 = engine_->updateState(state5, Action::move(1), Action::move(1));

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
  // Blissey has Soft-Boiled (Move 0, Status), Seismic Toss (Move 1, Damage)

  // Alakazam uses Encore on Turn 2 after Blissey uses Soft-Boiled on Turn 1
  // Turn 1: Blissey uses Soft-Boiled (Move 1), Alakazam uses Psychic
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::move(1));
  // Turn 2: Alakazam uses Encore, Blissey uses Soft-Boiled
  auto turn2 = engine_->updateState(turn1.where1(), Action::move(0), Action::wait());
  auto state = turn2.where1();

  // Now Alakazam uses Taunt on Turn 3.
  auto turn3 = engine_->updateState(state, Action::move(2), Action::wait());
  auto env3 = turn3.where1();
  EXPECT_GT(env3.teammate(1, 0).status().cTeammate.encore_duration, 0);

  // Now Blissey is Encored into Soft-Boiled AND Taunted.
  // Move 0: Seismic Toss (Damage) - Blocked by Encore (not Move 1)
  // Move 1: Soft-Boiled (Status) - Blocked by Taunt
  // Move 2: Toxic (Status) - Blocked by both

  EXPECT_FALSE(engine_->isValidAction(env3, Action::move(0), TEAM_B));
  EXPECT_FALSE(engine_->isValidAction(env3, Action::move(1), TEAM_B));
  EXPECT_FALSE(engine_->isValidAction(env3, Action::move(2), TEAM_B));

  // Check for Struggle
  EXPECT_TRUE(engine_->isValidAction(env3, Action::struggle(), TEAM_B));

  // Verify that updateState results in Struggle (or is blocked)
  auto turn4 = engine_->updateState(env3, Action::wait(), Action::move(1));
  auto env4 = turn4.where1();

  // Blissey should have been blocked (from Move 1 because of Taunt)
  EXPECT_TRUE(env4.wasBlocked(TEAM_B));
}

TEST_F(EncoreTest, EndsOnPPDepletion) {
  // Setup Blissey with 1 PP for Soft-Boiled
  auto turn1 = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  auto state1 = turn1.where1();
  state1.teammate(1, 0).getMV(Action::move(0)).setPP(1);

  // Alakazam encores
  auto turn2 = engine_->updateState(state1, Action::move(0), Action::wait());
  auto state2 = turn2.where1();

  EXPECT_GT(state2.teammate(1, 0).status().cTeammate.encore_duration, 0);

  // Blissey uses Soft-Boiled (Move 0), using up its last PP
  auto turn3 = engine_->updateState(state2, Action::wait(), Action::move(0));
  auto state3 = turn3.where1();

  // Encore should end because PP is 0
  EXPECT_EQ(state3.teammate(1, 0).status().cTeammate.encore_duration, 0);
}
