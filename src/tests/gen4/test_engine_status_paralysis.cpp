#include "engine_test.hpp"

class ParalysisStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Mew (Speed 100)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("thunder wave"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));

    // Team B: Gengar (Speed 110)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("shadow ball"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(ParalysisStatusTest, Test_AppliesParalysis) {
  auto initial_state = engine_->initialState();
  // Mew uses Thunder Wave, Gengar uses Shadow Ball
  auto results = engine_->updateState(initial_state, Action::move(0), Action::move(0));

  // Gengar should be paralyzed in the state where Thunder Wave hit
  // Thunder Wave has 100% accuracy, so it should be the most probable state
  auto paralyzed_state = results.where1Status(0);
  EXPECT_EQ(paralyzed_state.teammate(1, 0).getStatusAilment(), AIL_NV_PARALYSIS);
}

TEST_F(ParalysisStatusTest, Test_ParalysisReducesSpeed) {
  auto initial_state = engine_->initialState();

  // Turn 1: Mew uses Thunder Wave, Gengar uses Shadow Ball
  // Gengar (110) is faster than Mew (100), so Gengar should move first
  auto results1 = engine_->updateState(initial_state, Action::move(0), Action::move(0));
  auto state1 = results1.where1Status(0);
  EXPECT_TRUE(state1.flagsFor(1, ActorProxy::ALL_TEAMMATES).isMovedFirst());
  EXPECT_EQ(state1.teammate(1, 0).getStatusAilment(), AIL_NV_PARALYSIS);

  // Turn 2: Mew uses Psychic, Gengar uses Shadow Ball
  // Now Gengar is paralyzed, so Mew (100) should be faster than Gengar (110/4 = 27.5)
  auto results2 = engine_->updateState(state1, Action::move(1), Action::move(0));

  // Mew should move first in the most probable state where it hits
  auto state2 = results2.where1Hit(0);
  EXPECT_TRUE(state2.flagsFor(0, ActorProxy::ALL_TEAMMATES).isMovedFirst());
}

TEST_F(ParalysisStatusTest, Test_FullParalysis) {
  auto initial_state = engine_->initialState();

  // Setup: Get to a state where Gengar is already paralyzed
  auto results1 = engine_->updateState(initial_state, Action::move(0), Action::move(0));
  auto paralyzed_state = results1.where1Status(0);

  // Turn 2: Gengar tries to move (Shadow Ball), Mew waits
  auto results2 = engine_->updateState(paralyzed_state, Action::wait(), Action::move(0));

  // There should be a state where Gengar is blocked (25% chance)
  bool found_blocked = false;
  bool found_not_blocked = false;
  for (size_t i = 0; i < results2.size(); ++i) {
    auto env = results2.at(i);
    if (env.flagsFor(1, ActorProxy::ALL_TEAMMATES).isBlocked()) {
      found_blocked = true;
      EXPECT_NEAR(env.getProbability().to_double(), 0.25, 0.01);
    } else {
      found_not_blocked = true;
    }
  }
  EXPECT_TRUE(found_blocked);
  EXPECT_TRUE(found_not_blocked);
}
