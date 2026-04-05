#include "engine_test.hpp"

class InfatuationStatusTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Mew (Male)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("attract"))
          .addMove(pokedex_->move("psychic"))
          .setSex(SEX_MALE)
          .setLevel(100));

    // Team B: Snorlax (Female)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("strength"))
          .setSex(SEX_FEMALE)
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(InfatuationStatusTest, Test_AppliesInfatuation) {
  auto initial_state = engine_->initialState();
  // Mew uses Attract, Snorlax uses Strength
  // Since Attract is a status move with 100% accuracy, and genders are opposite, it should hit.
  auto results = engine_->updateState(initial_state, Action::move(0), Action::move(0));

  // Snorlax should be infatuated in the state where Attract hit
  auto infatuated_state = results.where1Status(0);
  EXPECT_EQ(infatuated_state.teammate(1, 0).status().cTeammate.infatuate, 1);
}

TEST_F(InfatuationStatusTest, Test_InfatuationBlocksMove) {
  auto initial_state = engine_->initialState();

  // Turn 1: Mew uses Attract, Snorlax waits
  auto results1 = engine_->updateState(initial_state, Action::move(0), Action::wait());
  auto infatuated_state = results1.where1Status(0);
  EXPECT_EQ(infatuated_state.teammate(1, 0).status().cTeammate.infatuate, 1);

  // Turn 2: Snorlax tries to move (Strength), Mew waits
  auto results2 = engine_->updateState(infatuated_state, Action::wait(), Action::move(0));

  // There should be a state where Snorlax is blocked (50% chance)
  bool found_blocked = false;
  FixType prob_not_blocked = FixType(0);
  for (size_t i = 0; i < results2.size(); ++i) {
    auto env = results2.at(i);
    if (env.flagsFor(1, ActorProxy::ALL_TEAMMATES).isBlocked()) {
      found_blocked = true;
      EXPECT_NEAR(env.getProbability().to_double(), 0.5, 0.01);
    } else if (env.flagsFor(1, ActorProxy::ALL_TEAMMATES).isHit()) {
      prob_not_blocked = prob_not_blocked + env.getProbability();
    }
  }
  EXPECT_TRUE(found_blocked);
  EXPECT_NEAR(prob_not_blocked.to_double(), 0.5, 0.01);
}
