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

    // Blissey: Pound (0), Softboiled (1), Toxic (2)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("pound"))        // 0
          .addMove(pokedex_->move("softboiled"))   // 1
          .addMove(pokedex_->move("toxic"))        // 2
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupFailsIfNoMoveUsed() {
    // T1: Alakazam(Disable), Blissey(Pound/Move 0). Alakazam goes first.
    return engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments setupPsychicSoftboiled() {
    // T1: Alakazam(Psychic), Blissey(Softboiled/Move 1)
    return engine_->updateState(engine_->initialState(), Action::move(1), Action::move(1));
  }

  PossibleEnvironments setupAppliesEffect() {
    // T2: Alakazam(Disable), Blissey(Pound/Move 0)
    auto t1 = setupPsychicSoftboiled();
    return engine_->updateState(t1.where1(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments setupAfterDisabled() {
    // T3: Alakazam(Disable), Blissey(Toxic/Move 2)
    auto applies = setupAppliesEffect();
    return engine_->updateState(applies.where1(), Action::move(0), Action::move(2));
  }

  PossibleEnvironments setupT3() {
    auto applies = setupAppliesEffect();
    return engine_->updateState(applies.where1(), Action::move(1), Action::move(0));
  }

  PossibleEnvironments setupT4() {
    return engine_->updateState(setupT3().where1(), Action::move(1), Action::move(0));
  }

  PossibleEnvironments setupT5() {
    return engine_->updateState(setupT4().where1(), Action::move(1), Action::move(0));
  }

  PossibleEnvironments setupT6() {
    auto t5 = setupT5();
    auto state5_filtered = t5.where1([](const ConstEnvironmentPossible& res) {
        return res.teammate(1, 0).status().cTeammate.disable_duration == 3;
    });
    return engine_->updateState(state5_filtered, Action::move(1), Action::move(0));
  }
};

TEST_F(DisableTest, FailsIfNoMoveUsed) {
  auto results = setupFailsIfNoMoveUsed();
  auto env = results.where1().getEnv();
  EXPECT_EQ(env.teammate(1, 0).status().cTeammate.disable_duration, 0);
}

TEST_F(DisableTest, AppliesEffect) {
  auto results = setupAppliesEffect();
  auto env = results.where1().getEnv();
  auto teamStatus = env.teammate(1, 0).status();

  // Blissey used Softboiled (Move 1) in T1.
  // Disable should disable Move 1.
  EXPECT_GT(teamStatus.cTeammate.disable_duration, 0);
  EXPECT_EQ(teamStatus.cTeammate.disable_action, 1);
}

TEST_F(DisableTest, RestrictsMoves) {
  auto results = setupAppliesEffect();
  auto state = results.where1();

  // Blissey Disabled on Move 1 (Softboiled).

  // Try Softboiled (Move 1) - Should be INVALID
  EXPECT_FALSE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(1)));
  // Try Seismic Toss (Move 0) - Should be VALID
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(0)));
  // Try Toxic (Move 2) - Should be VALID
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(2)));
}

TEST_F(DisableTest, FailsIfAlreadyDisabled) {
  auto applies = setupAppliesEffect();
  auto state2 = applies.where1();
  auto durationAfterTurn2 = state2.teammate(1, 0).status().cTeammate.disable_duration;

  // Turn 3: Alakazam uses Disable again.
  auto results = setupAfterDisabled();
  auto state3 = results.where1();

  // Should decrement (end of turn) but NOT reset (set fails).
  EXPECT_EQ(state3.teammate(1, 0).status().cTeammate.disable_duration, durationAfterTurn2 - 1);
}

TEST_F(DisableTest, ProbabilisticDuration) {
  // Similar logic to Encore logic

  // T2 end: 6 (setupAppliesEffect)
  auto applies = setupAppliesEffect();
  EXPECT_EQ(applies.where1().teammate(1, 0).status().cTeammate.disable_duration, 6);

  // T3 end: 5
  auto t3 = setupT3();
  EXPECT_EQ(t3.where1().teammate(1, 0).status().cTeammate.disable_duration, 5);

  // T4 end: 4
  auto t4 = setupT4();
  EXPECT_EQ(t4.where1().teammate(1, 0).status().cTeammate.disable_duration, 4);

  // T5 end: 3 (75%) or 0 (25%)
  auto t5 = setupT5();
  EXPECT_GE(t5.size(), 2);

  // T6 end from state5_filtered (dur 3): 2 (66%) or 0 (33%)
  auto t6 = setupT6();
  EXPECT_GE(t6.size(), 2);

  bool foundEnded = false;
  bool foundContinued = false;

  for (size_t i = 0; i < t6.size(); ++i) {
    auto res = t6.at(i);
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
