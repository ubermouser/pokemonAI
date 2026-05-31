#include "engine_test.hpp"

class TauntTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("steelix"))
          .addMove(pokedex_->move("taunt"))
          .addMove(pokedex_->move("strength")) // damage
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu"))); // backup

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("shuckle"))
          .addMove(pokedex_->move("toxic")) // status
          .addMove(pokedex_->move("strength")) // damage
          .setLevel(100));

    env_nv = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(env_nv);
  }

  PossibleEnvironments setupTauntApplied() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupTauntTurn2() {
    return engine_->updateState(
        setupTauntApplied().where1(), Action::move(1), Action::move(1));
  }

  PossibleEnvironments setupTauntTurn3() {
    return engine_->updateState(
        setupTauntTurn2().where1(), Action::move(1), Action::move(1));
  }

  PossibleEnvironments setupTauntTurn4() {
    return engine_->updateState(
        setupTauntTurn3().where1(), Action::move(1), Action::move(1));
  }

  PossibleEnvironments setupTauntTurn5() {
    return engine_->updateState(
        setupTauntTurn4().where1(), Action::move(1), Action::move(1));
  }

  PossibleEnvironments setupPreemptSameTurn() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(0));
  }

  std::shared_ptr<EnvironmentNonvolatile> env_nv;
};

TEST_F(TauntTest, AppliesEffect) {
  auto results = setupTauntApplied();
  auto env = results.where1().getEnv();

  // Shuckle should have taunt duration
  EXPECT_GT(env.teammate(1, 0).status().taunt_duration, 0);
  // Steelix should not
  EXPECT_EQ(env.teammate(0, 0).status().taunt_duration, 0);
}

TEST_F(TauntTest, PreventsStatusMoves) {
  auto results = setupTauntApplied();
  auto state = results.where1();

  // Shuckle tries to use Toxic (Move 0, Status) - Should be invalid
  EXPECT_FALSE(
      engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(0)));

  // Shuckle tries to use Strength (Move 1, Physical) - Should be valid
  EXPECT_TRUE(
      engine_->isValidAction(state, Actor(TEAM_B, 0), Action::move(1)));
}

TEST_F(TauntTest, PreemptsStatusMoveSameTurn) {
  auto results = setupPreemptSameTurn();
  auto final_env = results.where1().getEnv();

  // 1. Shuckle should be taunted
  EXPECT_GT(final_env.teammate(1, 0).status().taunt_duration, 0);

  // 2. Steelix should NOT be poisoned (Toxic should have failed)
  EXPECT_EQ(final_env.teammate(0, 0).getStatusAilment(), AIL_NV_NONE);
}

TEST_F(TauntTest, TauntReported) {
  auto results = setupTauntApplied();
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), results.where1(), false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("shuckle was taunted") != std::string::npos);
}

TEST_F(TauntTest, ProbabilisticDuration_DecrementsBeforeBranching) {
  auto results1 = setupTauntApplied();
  auto state1 = results1.where1();
  EXPECT_EQ(state1.teammate(1, 0).status().taunt_duration, 5);

  auto results2 = setupTauntTurn2();
  auto state2 = results2.where1();
  EXPECT_EQ(state2.teammate(1, 0).status().taunt_duration, 4);

  auto results3 = setupTauntTurn3();
  auto state3 = results3.where1();
  EXPECT_EQ(state3.teammate(1, 0).status().taunt_duration, 3);

  auto results4 = setupTauntTurn4();
  auto state4 = results4.where1();
  EXPECT_EQ(state4.teammate(1, 0).status().taunt_duration, 2);
}

TEST_F(TauntTest, ProbabilisticDuration_BranchesByTurn5) {
  // At Turn 5: Shuckle's preempt runs. Since duration is 2, it splits (1/3 ends, 2/3 continues).
  // Continued state will have duration 1. Ended state will have duration 0.
  auto results5 = setupTauntTurn5();
  EXPECT_GE(results5.size(), 2);

  auto state5_ended = results5.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(1, 0).status().taunt_duration == 0;
  });
  auto state5_continued = results5.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(1, 0).status().taunt_duration != 0;
  });

  EXPECT_EQ(state5_ended.teammate(1, 0).status().taunt_duration, 0);
  EXPECT_EQ(state5_continued.teammate(1, 0).status().taunt_duration, 1);
}

TEST_F(TauntTest, ProbabilisticDuration_ExpiresByTurn6) {
  auto results5 = setupTauntTurn5();
  auto state5_filtered = results5.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(1, 0).status().taunt_duration == 1;
  });

  // At Turn 6: Shuckle uses Strength (Move 1). Taunt duration should expire to 0.
  auto results6 = engine_->updateState(state5_filtered.getEnv(), Action::move(1), Action::move(1));
  auto state6 = results6.where1();
  EXPECT_EQ(state6.teammate(1, 0).status().taunt_duration, 0);
}
