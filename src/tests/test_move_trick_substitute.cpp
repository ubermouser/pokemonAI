#include "engine_test.hpp"

class TrickSubstituteTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("kadabra"))
          .addMove(pokedex_->move("trick"))
          .addMove(pokedex_->move("recover"))
          .setInitialItem(pokedex_->item("choice specs"))
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("substitute"))
          .addMove(pokedex_->move("softboiled"))
          .setInitialItem(pokedex_->item("leftovers"))
          .setLevel(100));

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));
  }
};

TEST_F(TrickSubstituteTest, fails_against_substitute) {
  // Turn 1:
  // Team A (Kadabra) uses Recover (Move 1).
  // Team B (Blissey) uses Substitute (Move 0).
  // Kadabra is faster (105 base speed vs Blissey 55), so Kadabra moves first.

  auto turn1 = engine_->updateState(
    engine_->initialState(), Action::move(1), Action::move(0));

  // Verify Substitute is up
  auto env1 = turn1.at(0).getEnv();
  EXPECT_GT(env1.getTeam(1).teammate(0).status().cTeammate.substitute, 0);

  // Turn 2:
  // Team A (Kadabra) uses Trick (Move 0).
  // Team B (Blissey) uses Softboiled (Move 1).

  auto turn2 = engine_->updateState(
    turn1.at(0), Action::move(0), Action::move(1));

  auto env2 = turn2.at(0).getEnv();

  // Verify Items
  // If Trick failed, items should be unchanged.
  // Kadabra should have Choice Specs.
  // Blissey should have Leftovers.

  EXPECT_EQ(env2.getTeam(0).getPKV().getItem().getName(), "choice specs");
  EXPECT_EQ(env2.getTeam(1).getPKV().getItem().getName(), "leftovers");
}

TEST_F(TrickSubstituteTest, succeeds_without_substitute) {
  // Turn 1:
  // Team A (Kadabra) uses Trick (Move 0).
  // Team B (Blissey) uses Softboiled (Move 1) (No Substitute).

  auto turn1 = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(1));

  auto env1 = turn1.at(0).getEnv();

  // Verify Items Swapped
  EXPECT_EQ(env1.getTeam(0).getPKV().getItem().getName(), "leftovers");
  EXPECT_EQ(env1.getTeam(1).getPKV().getItem().getName(), "choice specs");
}
