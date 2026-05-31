#include "engine_test.hpp"

class HazeTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Setup: Team A has Crobat with Haze. Team B has Charmander.
    // Both have boosted stats.

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("crobat"))
            .addMove(pokedex_->move("haze"))
            .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("charmander"))
            .addMove(pokedex_->move("ember")) // Dummy move
            .setLevel(100));

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

    auto initialEnv = engine_->initialState();
    EnvironmentVolatileData mutableData = initialEnv.data();

    // Set boosts for Team A (Crobat)
    mutableData.teams[0].activeVolatiles[0].boosts.B_ATK = 2;
    mutableData.teams[0].activeVolatiles[0].boosts.B_DEF = -2;
    mutableData.teams[0].activeVolatiles[0].boosts.B_ACC = 1;
    mutableData.teams[0].activeVolatiles[0].boosts.B_CHT = 1;

    // Set boosts for Team B (Charmander)
    mutableData.teams[1].activeVolatiles[0].boosts.B_SPA = 3;
    mutableData.teams[1].activeVolatiles[0].boosts.B_SPE = -1;

    // Create new environment with modified data
    EnvironmentVolatile modifiedEnv(initialEnv.nv(), mutableData);

    // Run the turn. Team A uses Haze (move 0). Team B waits.
    haze_result = engine_->updateState(
        modifiedEnv, Action::move(0), Action::wait());
  }

  PossibleEnvironments haze_result;
};

TEST_F(HazeTest, ResetsStats) {
  // Check the resulting state
  // We expect 1 hit (Haze always hits)
  ASSERT_TRUE(haze_result.size() > 0);

  auto state = haze_result.where1Hit(0);

  // Check Team A (Crobat) stats
  EXPECT_EQ(state.teammate(0, 0).getBoost(FV_ATTACK), 0);
  EXPECT_EQ(state.teammate(0, 0).getBoost(FV_DEFENSE), 0);
  EXPECT_EQ(state.teammate(0, 0).getBoost(FV_ACCURACY), 0);
  EXPECT_EQ(state.teammate(0, 0).getBoost(FV_CRITICALHIT), 0);

  // Check Team B (Charmander) stats
  EXPECT_EQ(state.teammate(1, 0).getBoost(FV_SPATTACK), 0);
  EXPECT_EQ(state.teammate(1, 0).getBoost(FV_SPEED), 0);

  // Verify other stats are 0 (default)
  EXPECT_EQ(state.teammate(0, 0).getBoost(FV_SPATTACK), 0);
  EXPECT_EQ(state.teammate(1, 0).getBoost(FV_DEFENSE), 0);
}
