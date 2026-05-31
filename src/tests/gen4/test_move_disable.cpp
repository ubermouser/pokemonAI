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
    return engine_->updateState(
        engine_->initialState(), Action::move(1), Action::move(1));
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

  PossibleEnvironments setupTurnN(int n) {
    if (n <= 2) return setupAppliesEffect();
    auto prev = setupTurnN(n - 1);
    auto state = prev.where1([](const ConstEnvironmentPossible& res) {
        return res.teammate(1, 0).status().disable_duration > 0;
    });
    return engine_->updateState(state, Action::move(2), Action::wait());
  }

  FixType accumulateProbability(
      const PossibleEnvironments& results,
      std::function<bool(const ConstEnvironmentPossible&)> predicate) {
    FixType sum(0);
    for (auto env : results.where(predicate)) { sum += env.getProbability(); }
    return sum;
  }
};

TEST_F(DisableTest, FailsIfNoMoveUsed) {
  auto results = setupFailsIfNoMoveUsed();
  auto env = results.where1().getEnv();
  EXPECT_EQ(env.teammate(1, 0).status().disable_duration, 0);
}

TEST_F(DisableTest, AppliesEffect) {
  auto results = setupAppliesEffect();
  auto env = results.where1().getEnv();
  auto teamStatus = env.teammate(1, 0).status();

  // Blissey used Softboiled (Move 1) in T1.
  // Disable should disable Move 1.
  EXPECT_GT(teamStatus.disable_duration, 0);
  EXPECT_EQ(teamStatus.disable_action, 1);
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
  auto durationAfterTurn2 = state2.teammate(1, 0).status().disable_duration;

  // Turn 3: Alakazam uses Disable again.
  auto results = setupAfterDisabled();
  auto state3 = results.where1();

  // Should decrement (end of turn) but NOT reset (set fails).
  EXPECT_EQ(state3.teammate(1, 0).status().disable_duration, durationAfterTurn2 - 1);
}


TEST_F(DisableTest, Duration6) {
  auto res = setupTurnN(2);
  FixType prob = accumulateProbability(res, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 6;
  });
  EXPECT_NEAR(prob.to_double(), 0.8, 1e-5);  // disable has a 20% chance to miss
}


TEST_F(DisableTest, Duration5) {
  auto res = setupTurnN(3);
  FixType prob = accumulateProbability(res, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 5;
  });
  EXPECT_EQ(prob.to_double(), 1.0);
}


TEST_F(DisableTest, Duration4) {
  auto res = setupTurnN(4);
  FixType prob = accumulateProbability(res, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 4;
  });
  EXPECT_EQ(prob.to_double(), 1.0);
}


TEST_F(DisableTest, Duration3) {
  // T5 end: 3 (75%) or 0 (25%)
  auto results = setupTurnN(5);

  FixType probContinued = accumulateProbability(results, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 3;
  });
  FixType probEnded = accumulateProbability(results, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 0;
  });

  EXPECT_NEAR(probContinued.to_double(), (3.0 / 4.0), 1e-5);
  EXPECT_NEAR(probEnded.to_double(), (1.0 / 4.0), 1e-5);
}


TEST_F(DisableTest, Duration2) {
  // T6 end from state5_filtered (dur 3): 2 (66%) or 0 (33%)
  auto results = setupTurnN(6);

  FixType probContinued = accumulateProbability(results, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 2;
  });
  FixType probEnded = accumulateProbability(results, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 0;
  });

  EXPECT_NEAR(probContinued.to_double(), (2.0 / 3.0), 1e-5);
  EXPECT_NEAR(probEnded.to_double(), (1.0 / 3.0), 1e-5);
}


TEST_F(DisableTest, Duration1) {
  // T7 end from state6_filtered (dur 2): 1 (50%) or 0 (50%)
  auto results = setupTurnN(7);

  FixType probContinued = accumulateProbability(results, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 1;
  });
  FixType probEnded = accumulateProbability(results, [](auto env) {
    return env.teammate(1, 0).status().disable_duration == 0;
  });

  EXPECT_NEAR(probContinued.to_double(), (1.0 / 2.0), 1e-5);
  EXPECT_NEAR(probEnded.to_double(), (1.0 / 2.0), 1e-5);
}