#include <gtest/gtest.h>

#include <memory>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/pkai.h"
#include "pokemonai/team_volatile.h"


class MoveTest : public ::testing::Test {
protected:
  void SetUp() override {
    verbose = 4;
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
};

class SuicideMoveTest : public MoveTest {
protected:
  void SetUp() override {
    MoveTest::SetUp();

    auto team_a_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("electrode"))
          .addMove(pokedex_->getMoves().at("explosion"))
          .addMove(pokedex_->getMoves().at("selfdestruct"))
          .setLevel(100));
    auto team_b_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("pikachu"))
          .addMove(pokedex_->getMoves().at("volt tackle"))
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team_a_nv, team_b_nv, true);
    engine_->setEnvironment(environment_nv);
  }

  EnvironmentNonvolatile environment_nv;
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
