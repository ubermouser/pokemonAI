#include "engine_test.hpp"


class TrickTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("kadabra"))
          .addMove(pokedex_->move("trick"))
          .addMove(pokedex_->move("recover"))
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
          .addMove(pokedex_->move("substitute"))
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

TEST_F(TrickTest, fails_against_substitute) {
  // Turn 1:
  // Team A (Kadabra) uses Recover (Move 1).
  // Team B (Blissey) uses Substitute (Move 2).
  // Kadabra is faster (105 base speed vs Blissey 55), so Kadabra moves first.

  auto turn1 = engine_->updateState(
    engine_->initialState(), Action::move(1), Action::move(2));

  // Verify Substitute is up
  auto env1 = turn1.at(0).getEnv();
  EXPECT_GT(env1.getTeam(1).teammate(0).status().cTeammate.substitute, 0);

  // Turn 2:
  // Team A (Kadabra) uses Trick (Move 0).
  // Team B (Blissey) uses Softboiled (Move 0).

  auto turn2 = engine_->updateState(
    turn1.at(0), Action::move(0), Action::move(0));

  auto env2 = turn2.at(0).getEnv();

  // Verify Items
  // If Trick failed, items should be unchanged.
  // Kadabra should have Choice Specs.
  // Blissey should have Leftovers.

  EXPECT_EQ(env2.getTeam(0).getPKV().getItem().getName(), "choice specs");
  EXPECT_EQ(env2.getTeam(1).getPKV().getItem().getName(), "leftovers");
}
