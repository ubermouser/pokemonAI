#include "engine_test.hpp"


class FakeOutTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("fake out"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("rotom"))
          .addMove(pokedex_->move("thunderbolt"))
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mewtwo"))
          .addMove(pokedex_->move("reflect"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments turn1FakeOut() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments turn1Psychic() {
    return engine_->updateState(
        engine_->initialState(), Action::move(1), Action::move(0));
  }

  PossibleEnvironments turn2FakeOutAfterPsychic() {
    auto turn1 = turn1Psychic();
    return engine_->updateState(
        turn1.where1(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments turn2SwitchToRotom() {
    auto turn1 = turn1FakeOut();
    return engine_->updateState(
        turn1.where1Hit(TEAM_A), Action::swap(1), Action::move(0));
  }

  PossibleEnvironments turn3SwitchToMew() {
    auto turn2 = turn2SwitchToRotom();
    return engine_->updateState(
        turn2.where1(), Action::swap(0), Action::move(0));
  }

  PossibleEnvironments turn4FakeOutAfterSwitch() {
    auto turn3 = turn3SwitchToMew();
    return engine_->updateState(
        turn3.where1(), Action::move(0), Action::move(0));
  }
};


TEST_F(FakeOutTest, SucceedsOnFirstTurn) {
  auto result = turn1FakeOut();
  auto state = result.where1Hit(TEAM_A);

  // Fake Out should deal damage (Mewtwo HP < 100%)
  EXPECT_LT(state.teammate(TEAM_B, 0).getPercentHP(), 1.0);

  // Mewtwo should be blocked due to flinching
  EXPECT_TRUE(state.flagsFor(TEAM_B).isBlocked());
}


TEST_F(FakeOutTest, FailsOnSecondTurn) {
  auto turn1 = turn1Psychic();
  auto state1 = turn1.where1();

  auto turn2 = turn2FakeOutAfterPsychic();
  auto state2 = turn2.where1();

  // The move should be blocked
  EXPECT_TRUE(state2.flagsFor(Actor(TEAM_A, 0)).isBlocked());

  // Mewtwo should not have taken additional damage from Fake Out
  EXPECT_EQ(state2.teammate(TEAM_B, 0).getPercentHP(), state1.teammate(TEAM_B, 0).getPercentHP());

  // Mewtwo should not be blocked (not flinched)
  EXPECT_FALSE(state2.flagsFor(TEAM_B).isBlocked());
}


TEST_F(FakeOutTest, SucceedsAgainAfterSwitching) {
  auto turn3 = turn3SwitchToMew();
  auto state3 = turn3.where1();

  // Get Mewtwo's HP before Fake Out
  double hp_before = state3.teammate(TEAM_B, 0).getPercentHP();

  auto turn4 = turn4FakeOutAfterSwitch();
  auto state4 = turn4.where1Hit(TEAM_A);

  // Fake Out should deal damage again (HP decreased)
  EXPECT_LT(state4.teammate(TEAM_B, 0).getPercentHP(), hp_before);

  // Mewtwo should flinch again
  EXPECT_TRUE(state4.flagsFor(TEAM_B).isBlocked());
}
