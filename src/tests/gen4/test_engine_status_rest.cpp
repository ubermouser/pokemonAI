#include "engine_test.hpp"

class RestStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Mew
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("rest"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    // Team B: Snorlax
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupRestFailed() {
    return engine_->updateState(engine_->initialState(), Action::moveAlly(0, 0), Action::wait());
  }

  PossibleEnvironments setupMewDamaged() {
    return engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments setupRestApplied() {
    auto results = setupMewDamaged();
    return engine_->updateState(
        results.where1().getEnv(),
        Action::moveAlly(0, 0),
        Action::wait());
  }

  PossibleEnvironments setupRestTurn2() {
    auto results = setupRestApplied();
    return engine_->updateState(
        results.where1().getEnv(),
        Action::move(1),
        Action::wait());
  }

  PossibleEnvironments setupRestTurn3() {
    auto results = setupRestTurn2();
    return engine_->updateState(
        results.where1().getEnv(),
        Action::move(1),
        Action::wait());
  }

  PossibleEnvironments setupRestTurn4() {
    auto results = setupRestTurn3();
    return engine_->updateState(
        results.where1().getEnv(),
        Action::move(1),
        Action::wait());
  }
};

TEST_F(RestStatusTest, Test_RestMoveAndTwoTurns) {
    // Turn 1: Mew is at full HP, Rest should fail
    auto results_fail = setupRestFailed();
    auto env_fail = results_fail.where1();
    EXPECT_EQ((uint32_t)env_fail.teammate(0, 0).getStatusAilment(), 0U);
    EXPECT_EQ(env_fail.teammate(0, 0).getHP(), (uint32_t)env_fail.teammate(0, 0).nv().getMaxHP());

    // Setup: Damage Mew by having Snorlax hit it
    auto setup_results = setupMewDamaged();
    auto setup_state = setup_results.where1();
    EXPECT_LT(setup_state.teammate(0, 0).getHP(), (uint32_t)setup_state.teammate(0, 0).nv().getMaxHP());

    // Turn 1: Mew uses Rest
    auto results1 = setupRestApplied();
    auto env1 = results1.where1();
    
    // Status should be AIL_NV_REST_3T (7)
    EXPECT_EQ((uint32_t)env1.teammate(0, 0).getStatusAilment(), AIL_NV_REST_3T);
    EXPECT_EQ(env1.teammate(0, 0).getHP(), (uint32_t)env1.teammate(0, 0).nv().getMaxHP());

    // Turn 2: Mew should be blocked and status decremented to 6 (AIL_NV_REST_2T)
    auto results2 = setupRestTurn2();
    auto env2 = results2.where1();
    EXPECT_EQ((uint32_t)env2.teammate(0, 0).getStatusAilment(), AIL_NV_REST_2T);
    // Verify Mew was blocked (Team B Snorlax HP should be full)
    EXPECT_EQ(env2.teammate(1, 0).getHP(), (uint32_t)env2.teammate(1, 0).nv().getMaxHP());

    // Turn 3: Mew should be blocked and status decremented to 5 (AIL_NV_REST_1T)
    auto results3 = setupRestTurn3();
    auto env3 = results3.where1();
    EXPECT_EQ((uint32_t)env3.teammate(0, 0).getStatusAilment(), AIL_NV_REST_1T);

    // Turn 4: Mew should wake up (status cleared) and be able to move
    auto results4 = setupRestTurn4();
    auto env4 = results4.where1();
    EXPECT_EQ((uint32_t)env4.teammate(0, 0).getStatusAilment(), AIL_NV_REST_0T);
    // Verify Mew actually hit on Turn 4 (Team B Snorlax HP should be reduced)
    EXPECT_LT(env4.teammate(1, 0).getHP(), (uint32_t)env4.teammate(1, 0).nv().getMaxHP());
}

TEST_F(RestStatusTest, Test_StateTransitionPrinterRest) {
    // Setup: Damage Mew by having Snorlax hit it
    auto setup_results = setupMewDamaged();
    auto setup_state = setup_results.where1();

    // Turn 1: Mew uses Rest
    auto results1 = setupRestApplied();
    auto env1 = results1.where1();
    
    std::string output = StateTransitionPrinter::printString(
        setup_state,
        env1,
        /*withStyle=*/false);
    EXPECT_TRUE(output.find("fell asleep!") != std::string::npos)
        << "Expected 'fell asleep!' in printer output: " << output;
}
