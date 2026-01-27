#include "engine_test.hpp"


class ClearBodyTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Setup 1: Clear Body (Metagross) vs Intimidate/Growl User (Charmander)
    auto team_a_clearbody = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("metagross"))
          .setAbility(pokedex_->ability("clear body"))
          .addMove(pokedex_->move("zen headbutt"))
          .setLevel(100));

    auto team_b_debuffer = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("growl"))
          .setLevel(100));

    auto env_clearbody = EnvironmentNonvolatile(team_a_clearbody, team_b_debuffer, true);
    engine_->setEnvironment(env_clearbody);
    setup_clearbody = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));

    auto env_control = EnvironmentNonvolatile(team_b_debuffer, team_b_debuffer, true);
    engine_->setEnvironment(env_control);
    setup_control = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments setup_clearbody;
  PossibleEnvironments setup_control;
};


TEST_F(ClearBodyTest, PreventsStatDrop) {
  auto env = setup_clearbody.where1().getEnv();
  // Verify Metagross starts with 0 boost
  EXPECT_EQ(env.getTeam(0).getPKV().getBoost(FV_ATTACK), 0);

  auto metagross_after_growl =
      setup_clearbody.where1().getEnv().getTeam(0).getPKV();
  EXPECT_EQ(metagross_after_growl.getBoost(FV_ATTACK), 0); // Should be unchanged
}


TEST_F(ClearBodyTest, StatDropOccursWithoutAbility) {
  auto charmander_after_growl =
      setup_control.where1().getEnv().getTeam(0).getPKV();
  EXPECT_EQ(charmander_after_growl.getBoost(FV_ATTACK), -1); // Should be lowered
}
