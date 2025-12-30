#include "engine_test.hpp"

class LevitateTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

    // Team A: Sandshrew with Earthquake
    team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("sandshrew"))
          .addMove(pokedex_->move("earthquake"))
          .setLevel(50));

    // Team B with Levitate: Gastly
    team_b_levitate = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gastly"))
          .setAbility(pokedex_->ability("levitate"))
          .addMove(pokedex_->move("shadow ball"))
          .setLevel(50));

    // Team B without Levitate: Grimer
    team_b_no_levitate = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("grimer"))
          .setAbility(pokedex_->ability("sticky hold"))
          .addMove(pokedex_->move("poison jab"))
          .setLevel(50));
  }

  TeamNonVolatile team_a;
  TeamNonVolatile team_b_levitate;
  TeamNonVolatile team_b_no_levitate;
};

TEST_F(LevitateTest, ImmunityToGroundMoves) {
  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_levitate, true));

  // Sandshrew uses Earthquake on Gastly
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  auto gastly = turn1.at(0).getEnv().getTeam(TEAM_B).getPKV();

  // Should take no damage (HP equal to Max HP)
  EXPECT_EQ(gastly.getHP(), gastly.nv().getMaxHP());
}

TEST_F(LevitateTest, VulnerabilityWithoutLevitate) {
  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b_no_levitate, true));

  // Sandshrew uses Earthquake on Grimer
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  auto grimer = turn1.at(0).getEnv().getTeam(TEAM_B).getPKV();

  // Should take damage
  EXPECT_LT(grimer.getHP(), grimer.nv().getMaxHP());
}
