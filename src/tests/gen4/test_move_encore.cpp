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

    env_nv = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(env_nv);
  }

  PossibleEnvironments setupPsychicSoftboiled() {
    auto res_psychic_softboiled = engine_->updateState(
        engine_->initialState(), Action::move(1), Action::move(1));
    return res_psychic_softboiled;
  }

  PossibleEnvironments setupWaitSeismic() {
    auto res_wait_seismic = engine_->updateState(
        engine_->initialState(), Action::wait(), Action::move(0));
    return res_wait_seismic;
  }

  PossibleEnvironments setupEncoredSeismic() {
    auto psychic_softboiled = setupPsychicSoftboiled();
    auto res_encored_seismic = engine_->updateState(
        psychic_softboiled.where1(), Action::move(0), Action::move(0));
    return res_encored_seismic;
  }

  PossibleEnvironments setupAlreadyEncored() {
    // Turn 3: Blissey encored into Soft-Boiled, but already encored after pound
    auto results = setupEncoredSeismic();
    auto res_after_encored = engine_->updateState(
        results.where1(), Action::move(0), Action::move(1));
    return res_after_encored;
  }

  PossibleEnvironments setupAppliesEffect() {
    auto psychic_softboiled = setupPsychicSoftboiled();
    auto res_applies_effect = engine_->updateState(
        psychic_softboiled.where1(), Action::move(0), Action::move(1));
    return res_applies_effect;
  }

  PossibleEnvironments setupRestrictsMoves() {
    auto wait_seismic = setupWaitSeismic();
    auto res_restricts_moves = engine_->updateState(
        wait_seismic.where1(), Action::move(0), Action::wait());
    return res_restricts_moves;
  }

  PossibleEnvironments setupEncoreWait() {
    auto psychic_softboiled = setupPsychicSoftboiled();
    auto res_encore_wait = engine_->updateState(
        psychic_softboiled.where1(), Action::move(0), Action::wait());
    return res_encore_wait;
  }

  PossibleEnvironments setupEncoreTaunt() {
    // Encored Blissey is taunted while using softboiled. It must struggle
    auto res_encore_wait = setupEncoreWait();
    auto res_encore_taunt = engine_->updateState(
        res_encore_wait.where1(), Action::move(2), Action::move(1));
    return res_encore_taunt;
  }

  PossibleEnvironments setupProbabilisticTurn3() {
    return engine_->updateState(
        setupAppliesEffect().where1().getEnv(),
        Action::move(1),
        Action::move(1));
  }

  PossibleEnvironments setupProbabilisticTurn4() {
    return engine_->updateState(
        setupProbabilisticTurn3().where1().getEnv(),
        Action::move(1),
        Action::move(1));
  }

  PossibleEnvironments setupProbabilisticTurn5() {
    return engine_->updateState(
        setupProbabilisticTurn4().where1().getEnv(),
        Action::move(1),
        Action::move(1));
  }

  PossibleEnvironments setupProbabilisticTurn6(
      const ConstEnvironmentPossible& state5) {
    return engine_->updateState(
        state5.getEnv(), Action::move(1), Action::move(1));
  }

  PossibleEnvironments setupPPDepletion() {
    auto results1 = setupWaitSeismic();
    auto state_pp_mod = results1.where1();
    state_pp_mod.teammate(1, 0).getMV(Action::move(0)).setPP(1);
    return engine_->updateState(
        state_pp_mod.getEnv(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupPPDepleted() {
    auto results_depletion = setupPPDepletion();
    return engine_->updateState(
        results_depletion.where1().getEnv(), Action::wait(), Action::move(0));
  }

  std::shared_ptr<const EnvironmentNonvolatile> env_nv;
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

  auto results = setupEncoredSeismic();
  auto state2 = results.where1();

  // Verify that Blissey's action was preempted and it used Soft-Boiled (Move 1)
  // instead of Seismic Toss (Move 0).
  // Note: SetUp logic:
  // state_p_s: Alakazam(Psychic), Blissey(Softboiled/Move 1)
  // state_encored_seismic: Alakazam(Encore), Blissey(Seismic Toss/Move 0)
  // Blissey is encored into Softboiled (Move 1).
  EXPECT_TRUE(state2.flagsFor(TEAM_B).isBlocked());  // It was "blocked" and switched to encored move

  // Check that Blissey is now encored into Soft-Boiled (Move 1)
  auto teamStatus = state2.teammate(1, 0).status();
  EXPECT_GT(teamStatus.cTeammate.encore_duration, 0);
  EXPECT_EQ(teamStatus.cTeammate.encore_action, 1);
}


TEST_F(EncoreTest, FailsIfAlreadyEncored_DoesNotResetDuration) {
  // If Blissey is already encored, using Encore again should fail and not
  // reset the duration.

  auto results2 = setupEncoredSeismic();
  auto state2 = results2.where1();
  auto durationAfterTurn2 =
      state2.teammate(1, 0).status().cTeammate.encore_duration;
  EXPECT_GT(durationAfterTurn2, 0);

  // Turn 3: Blissey encored into Soft-Boiled.
  // duration should decrement due to the turn end, but NOT be reset by the
  // second Encore attempt.
  auto results3 = setupAlreadyEncored();
  auto state3 = results3.where1();

  auto teamStatus = state3.teammate(1, 0).status();
  // Duration should be durationAfterTurn2 - 1
  EXPECT_EQ(teamStatus.cTeammate.encore_duration, durationAfterTurn2 - 1);
}


TEST_F(EncoreTest, AppliesEffect) {
  // Turn 1: Blissey uses Soft-Boiled, Alakazam uses Psychic
  // Turn 2: Blissey uses Soft-Boiled, Alakazam uses Encore

  auto results = setupAppliesEffect();
  auto env = results.where1().getEnv();
  auto teamStatus = env.teammate(1, 0).status();

  // Blissey should be encored into Soft-Boiled (Move 1)
  EXPECT_GT(teamStatus.cTeammate.encore_duration, 0);
  EXPECT_EQ(teamStatus.cTeammate.encore_action, 1);
}


TEST_F(EncoreTest, RestrictsMoves_DisallowsNonEncoredMoves) {
  auto results = setupRestrictsMoves();
  auto state = results.where1();

  // Blissey tries to use Soft-Boiled (Move 1) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(1)));
}


TEST_F(EncoreTest, RestrictsMoves_AllowsEncoredMove) {
  auto results = setupRestrictsMoves();
  auto state = results.where1();

  // Blissey tries to use Seismic Toss (Move 0) - Should be valid
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(0)));
}


TEST_F(EncoreTest, ProbabilisticDuration_DecrementsBeforeBranching) {
  auto results3 = setupProbabilisticTurn3();
  auto state3 = results3.where1();
  EXPECT_EQ(state3.teammate(1, 0).status().cTeammate.encore_duration, 5);

  auto results4 = setupProbabilisticTurn4();
  auto state4 = results4.where1();
  EXPECT_EQ(state4.teammate(1, 0).status().cTeammate.encore_duration, 4);
}


TEST_F(EncoreTest, ProbabilisticDuration_BranchesByTurn5) {
  auto state_prob_5 = setupProbabilisticTurn5();
  EXPECT_GE(state_prob_5.size(), 2);
}


TEST_F(EncoreTest, ProbabilisticDuration_BranchesByTurn6) {
  auto state_prob_5 = setupProbabilisticTurn5();
  auto state5_filtered =
      state_prob_5.where1([](const ConstEnvironmentPossible& res) {
        return res.teammate(1, 0).status().cTeammate.encore_duration == 3;
      });

  auto results6 = setupProbabilisticTurn6(state5_filtered);
  EXPECT_GE(results6.size(), 2);

  bool foundEnded = false;
  bool foundContinued = false;

  for (size_t i = 0; i < results6.size(); ++i) {
    auto res = results6.at(i);
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


TEST_F(EncoreTest, EncoreAndTaunt_DisallowsAllMoves) {
  auto results3 = setupEncoreTaunt();
  auto env3 = results3.where1();
  EXPECT_GT(env3.teammate(1, 0).status().cTeammate.encore_duration, 0);

  // Now Blissey is Encored into Soft-Boiled (Move 1) AND Taunted.
  // Move 0: Seismic Toss (Damage) - Blocked by Encore (not Move 1)
  // Move 1: Soft-Boiled (Status) - Blocked by Taunt
  // Move 2: Toxic (Status) - Blocked by both

  EXPECT_FALSE(engine_->isValidAction(env3, Actor(TEAM_B, 0), Action::move(0)));
  EXPECT_FALSE(engine_->isValidAction(env3, Actor(TEAM_B, 0), Action::move(1)));
  EXPECT_FALSE(engine_->isValidAction(env3, Actor(TEAM_B, 0), Action::move(2)));
}


TEST_F(EncoreTest, EncoreAndTaunt_AllowsStatusMoveBeforeTaunt) {
  auto resultsBefore = setupEncoreWait();
  auto stateBefore = resultsBefore.where1();

  // At this point, Blissey is encored but NOT taunted.
  // Softboiled (Move 1) should be valid.
  EXPECT_TRUE(
      engine_->isValidAction(stateBefore, Actor(TEAM_B, 0), Action::move(1)));
}


TEST_F(EncoreTest, EncoreAndTaunt_AllowsStruggle) {
  auto resultsAfter = setupEncoreTaunt();
  auto envAfter = resultsAfter.where1();
  EXPECT_TRUE(
      engine_->isValidAction(envAfter, Actor(TEAM_B, 0), Action::struggle()));
}


TEST_F(EncoreTest, EncoreAndTaunt_TriggersStruggle) {
  auto resultsAfter = setupEncoreTaunt();
  auto envAfter = resultsAfter.where1();

  // Blissey should have been blocked (from Move 1 because of Taunt applied this
  // turn) and used Struggle
  EXPECT_TRUE(envAfter.flagsFor(Actor(TEAM_B, 0)).isBlocked());
}


TEST_F(EncoreTest, EndsOnPPDepletion_EndsDuration) {
  auto results_pp_depletion = setupPPDepleted();
  auto state3 = results_pp_depletion.where1();

  // Encore should end because PP is 0
  EXPECT_EQ(state3.teammate(1, 0).status().cTeammate.encore_duration, 0);
}
