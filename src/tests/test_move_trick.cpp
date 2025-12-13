#include "engine_test.hpp"


class TrickTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("kadabra"))
          .addMove(pokedex_->move("trick"))
          .setInitialItem(pokedex_->item("choice specs"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("alakazam"))
          .addMove(pokedex_->move("trick"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("softboiled"))
          .addMove(pokedex_->move("charm"))
          .setInitialItem(pokedex_->item("leftovers"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gastrodon"))
          .setAbility(pokedex_->ability("sticky hold"))
          .setInitialItem(pokedex_->item("choice band"))
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};


TEST_F(TrickTest, item_for_item) {
  auto trick_item = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::wait());

  auto final_env_v = trick_item.at(0).getEnv();
  EXPECT_EQ(final_env_v.getTeam(0).getPKV().getItem().getName(), "leftovers");
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "choice specs");
}


TEST_F(TrickTest, item_behavior_propagates) {
  auto trick_item = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(0));

  auto final_env_v = trick_item.at(0).getEnv();
  EXPECT_FALSE(engine_->isValidAction(final_env_v, Action::move(1), 1));
}


TEST_F(TrickTest, no_item_for_item) {
  auto trick_no_item = engine_->updateState(
    engine_->initialState(), Action::swap(1), Action::wait());
  auto final_trick = engine_->updateState(
    trick_no_item.at(0), Action::move(0), Action::wait());

  auto final_env_v = final_trick.at(0).getEnv();
  EXPECT_EQ(final_env_v.getTeam(0).getPKV().getItem().getName(), "leftovers");
  EXPECT_FALSE(final_env_v.getTeam(1).getPKV().hasItem());
}


TEST_F(TrickTest, item_for_no_item) {
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("blissey"))
        .addMove(pokedex_->move("softboiled"))
        .setLevel(100));
  engine_->setEnvironment(EnvironmentNonvolatile(environment_nv.getTeam(0), team_b, true));

  auto trick_item = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::wait());

  auto final_env_v = trick_item.at(0).getEnv();
  EXPECT_FALSE(final_env_v.getTeam(0).getPKV().hasItem());
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "choice specs");
}


TEST_F(TrickTest, sticky_hold_fails) {
  auto trick_item = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::swap(1));

  auto final_env_v = trick_item.at(0).getEnv();
  EXPECT_EQ(final_env_v.getTeam(0).getPKV().getItem().getName(), "choice specs");
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "choice band");
}
