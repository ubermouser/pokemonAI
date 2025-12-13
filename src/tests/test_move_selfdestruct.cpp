#include "engine_test.hpp"


class SuicideMoveTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();

    auto team_a_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("electrode"))
          .addMove(pokedex_->move("explosion"))
          .addMove(pokedex_->move("selfdestruct"))
          .setLevel(100));
    auto team_b_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu"))
          .addMove(pokedex_->move("volt tackle"))
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team_a_nv, team_b_nv, true);
    engine_->setEnvironment(environment_nv);
  }
};


TEST_F(SuicideMoveTest, Explosion) {
  auto possible_envs = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::wait());
  auto final_env_v = possible_envs.at(0).getEnv();

  EXPECT_EQ(final_env_v.getTeam(0).getPKV().getHP(), 0);
  EXPECT_LT(final_env_v.getTeam(1).getPKV().getPercentHP(), 0.5);
}


TEST_F(SuicideMoveTest, SelfDestruct) {
  auto possible_envs = engine_->updateState(
    engine_->initialState(), Action::move(1), Action::wait());
  auto final_env_v = possible_envs.at(0).getEnv();

  EXPECT_EQ(final_env_v.getTeam(0).getPKV().getHP(), 0);
  EXPECT_LT(final_env_v.getTeam(1).getPKV().getPercentHP(), 0.5);
}
