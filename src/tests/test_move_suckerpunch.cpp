#include "engine_test.hpp"

class SuckerPunchTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("spiritomb"))
          .addMove(pokedex_->move("sucker punch"))
          .addMove(pokedex_->move("will-o-wisp"))
          .addMove(pokedex_->move("shock wave"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("azelf"))
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(SuckerPunchTest, no_damage_against_status_effect_move) {
  auto suckerpunch_status = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(1));

  EXPECT_EQ(suckerpunch_status.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 1.);
  EXPECT_EQ(suckerpunch_status.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
}

TEST_F(SuckerPunchTest, no_damage_against_swapping_pokemon) {
  auto suckerpunch_move = engine_->updateState(engine_->initialState(), Action::move(0), Action::swap(1));

  EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 1.);
  EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(1).teammate(1).getPercentHP(), 1.);
  EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
}

TEST_F(SuckerPunchTest, full_damage_against_attacking_pokemon) {
  auto suckerpunch_dmg = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(2));

  EXPECT_LT(suckerpunch_dmg.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 0.75);
  EXPECT_EQ(suckerpunch_dmg.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
}
