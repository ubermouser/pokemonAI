#include "engine_test.hpp"

class PressureTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("starmie"))
          .addMove(pokedex_->move("water gun"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mewtwo"))
          .setAbility(pokedex_->ability("pressure"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(100));
    auto environment = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment);
    auto env_v = engine_->initialState();

    turn1_water_gun = engine_->updateState(
      env_v, Action::move(0), Action::wait());
  }

  PossibleEnvironments turn1_water_gun;
};

TEST_F(PressureTest, PressureIncreasesPPCost) {
  // Turn 1: Starmie uses water gun
  ASSERT_EQ(turn1_water_gun.size(), 2);
  auto starmie_after_water_gun = turn1_water_gun.at(0).getEnv().getTeam(TEAM_A).getPKV();
  EXPECT_EQ(starmie_after_water_gun.getMV(0).getPP(), 38);
}
