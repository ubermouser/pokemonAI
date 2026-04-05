#include "engine_test.hpp"

class FlinchStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Mew (Psychic) - Fast (100 Base Speed)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("iron head"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    // Team B: Snorlax (Normal) - Slow (30 Base Speed)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("tackle"))
          .addMove(pokedex_->move("iron head")) // For slow flinch test
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(FlinchStatusTest, Test_IronHeadCausesFlinch) {
    // Mew uses Iron Head on Snorlax. Snorlax uses Tackle.
    // Mew is faster and moves first.
    auto results = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));

    // Pick the state where the flinch (secondary effect) occurred.
    auto flinch_state = results.where1Status(TEAM_A);

    // Snorlax (Team B) should have been blocked from moving
    EXPECT_TRUE(flinch_state.flagsFor(TEAM_B, ActorProxy::ALL_TEAMMATES).isBlocked());

    // Verify Snorlax did no damage to Mew
    EXPECT_EQ(flinch_state.teammate(0, 0).getHP(), (uint32_t)flinch_state.teammate(0, 0).nv().getMaxHP());
}

TEST_F(FlinchStatusTest, Test_FlinchWearsOff) {
    // Turn 1: Mew uses Iron Head, Snorlax flinches
    auto results1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    auto flinch_state1 = results1.where1Status(TEAM_A);
    EXPECT_TRUE(flinch_state1.flagsFor(TEAM_B, ActorProxy::ALL_TEAMMATES).isBlocked());

    // Turn 2: Mew uses Psychic, Snorlax uses Tackle
    // Snorlax should NOT be flinching anymore.
    auto results2 = engine_->updateState(flinch_state1.getEnv(), Action::move(1), Action::move(0));
    auto env2 = results2.where1();

    // Snorlax should NOT be blocked
    EXPECT_FALSE(env2.flagsFor(TEAM_B, ActorProxy::ALL_TEAMMATES).isBlocked());

    // Snorlax should have hit Mew
    EXPECT_LT(env2.teammate(0, 0).getHP(), (uint32_t)env2.teammate(0, 0).nv().getMaxHP());
}

TEST_F(FlinchStatusTest, Test_SlowFlinchMoveDoesNotBlock) {
    // Mew (fast) uses Psychic, Snorlax (slow) uses Iron Head.
    // Mew moves first.
    auto results = engine_->updateState(engine_->initialState(), Action::move(1), Action::move(1));

    // In all states, Mew should NOT be blocked because it already moved.
    // Note: where1Status(TEAM_B) may fail here because states where flinch has no effect are merged.
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_FALSE(results.at(i).flagsFor(TEAM_A, ActorProxy::ALL_TEAMMATES).isBlocked());
    }

    // Pick a state where Mew hit Snorlax
    auto hit_state = results.where1Hit(TEAM_A);
    EXPECT_TRUE(hit_state.flagsFor(TEAM_A, ActorProxy::ALL_TEAMMATES).isHit());

    // Verify Mew can still move next turn
    auto results2 = engine_->updateState(hit_state.getEnv(), Action::move(1), Action::move(0));
    auto env2 = results2.where1();
    EXPECT_FALSE(env2.flagsFor(TEAM_A, ActorProxy::ALL_TEAMMATES).isBlocked());
}

TEST_F(FlinchStatusTest, Test_FlinchReported) {
    // Mew uses Iron Head on Snorlax
    auto results = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
    auto flinch_state = results.where1Status(TEAM_A);
    auto output = StateTransitionPrinter::printString(
      engine_->initialState(), flinch_state, false);

    SCOPED_TRACE(output);
    // Currently the engine reports "blocked" for flinch in reportHitResult
    EXPECT_TRUE(output.find("move was blocked") != std::string::npos);
}
