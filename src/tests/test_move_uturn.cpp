#include "engine_test.hpp"


class UTurnTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("scizor"))
          .addMove(pokedex_->move("u-turn"))
          .setInitialItem(pokedex_->item("life orb"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("torterra"))
          .addMove(pokedex_->move("stealth rock"))
          .setLevel(50));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);

    setup_swap = engine_->updateState(engine_->initialState(), Action::wait(), Action::swap(1));
    setup_sr = engine_->updateState(setup_swap.at(0), Action::wait(), Action::move(0));
    uturn_to_ally = engine_->updateState(
      setup_swap.at(0), Action::moveAlly(0, 1), Action::wait());
    uturn_to_ally_with_sr = engine_->updateState(
      setup_sr.at(0), Action::moveAlly(0, 1), Action::wait());
    swap_to_scyzor = engine_->updateState(
      uturn_to_ally.at(0), Action::wait(), Action::swap(0));
    uturn_no_ally = engine_->updateState(
      swap_to_scyzor.at(0), Action::wait(), Action::moveAlly(0, 0));
  }

  PossibleEnvironments setup_swap;
  PossibleEnvironments setup_sr;
  PossibleEnvironments uturn_to_ally;
  PossibleEnvironments uturn_to_ally_with_sr;
  PossibleEnvironments swap_to_scyzor;
  PossibleEnvironments uturn_no_ally;
};


TEST_F(UTurnTest, requires_pokemon_to_swap_if_ally_exists) {
  EXPECT_FALSE(engine_->isValidAction(setup_sr.at(0), Action::move(0), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(setup_sr.at(0), Action::moveAlly(0, 1), TEAM_A));
  EXPECT_FALSE(engine_->isValidAction(setup_sr.at(0), Action::moveAlly(0, 0), TEAM_A));
}


TEST_F(UTurnTest, can_still_be_used_without_swap_if_no_allies_exist) {
  EXPECT_FALSE(engine_->isValidAction(swap_to_scyzor.at(0), Action::move(0), TEAM_B));
  EXPECT_TRUE(engine_->isValidAction(swap_to_scyzor.at(0), Action::moveAlly(0, 0), TEAM_B));
  EXPECT_FALSE(engine_->isValidAction(swap_to_scyzor.at(0), Action::moveAlly(0, 1), TEAM_B));
}


TEST_F(UTurnTest, damages_enemy_and_swaps_to_ally) {
  // pp decremented
  EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 31);
  // item effect (life orb) applies
  EXPECT_NEAR(uturn_to_ally.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 0.9, 0.005);
  // ally has swapped out
  EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(0).getICPKV(), 1);
  EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(1).teammate(1).getPercentHP(), 0.); // enemy weakling deleted
}


TEST_F(UTurnTest, damages_enemy_and_swaps_to_ally_with_stealth_rock) {
  // life orb applies to attacking teammate
  EXPECT_NEAR(uturn_to_ally_with_sr.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 0.9, 0.005);
  // stealth-rock applies to entering teammate
  EXPECT_NEAR(uturn_to_ally_with_sr.at(0).getEnv().getTeam(0).teammate(1).getPercentHP(), 0.9375, 0.005);
}


TEST_F(UTurnTest, damages_enemy_but_doesnt_swap_if_no_allies_exist) {
  // pp decremented
  EXPECT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).teammate(0).getMV(0).getPP(), 31);
  // item effect (life orb) applies
  EXPECT_FLOAT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 0.9);
  // ally NOT swapped out
  EXPECT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).getICPKV(), 0);
}
