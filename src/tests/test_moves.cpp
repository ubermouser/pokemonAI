#include <gtest/gtest.h>

#include <memory>

#include "pokemonai/engine.h"
#include "pokemonai/game.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/planner_random.h"
#include "pokemonai/planner_max.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/pkai.h"
#include "pokemonai/team_volatile.h"

class MoveTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    PkCU::Config cfg;
    cfg.allowInvalidMoves = true;
    engine_ = std::make_unique<PkCU>(cfg);
  }

  void runTest(const std::string& moveName) {
    auto team_a_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("electrode"))
          .addMove(pokedex_->getMoves().at(moveName))
          .setLevel(100));
    auto team_b_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("pikachu"))
          .addMove(pokedex_->getMoves().at("volt tackle"))
          .setLevel(100));
    auto environment_nv = EnvironmentNonvolatile(team_a_nv, team_b_nv, true);

    engine_->setEnvironment(environment_nv);

    auto initial_env_v = engine_->initialState();

    auto pokemonA_nv = environment_nv.getTeam(0).teammate(0);
    auto pokemonB_nv = environment_nv.getTeam(1).teammate(0);

    auto pokemonA_v = initial_env_v.getTeam(0).getPKV();
    auto pokemonB_v = initial_env_v.getTeam(1).getPKV();

    EXPECT_EQ(pokemonA_v.getHP(), pokemonA_nv.getFV_base(FV_HITPOINTS));
    EXPECT_EQ(pokemonB_v.getHP(), pokemonB_nv.getFV_base(FV_HITPOINTS));

    Action actionA(Action::MOVE_0);
    Action actionB(Action::wait());

    auto possible_envs = engine_->updateState(initial_env_v, actionA, actionB);

    ASSERT_FALSE(possible_envs.empty());

    auto final_env_v = possible_envs.at(0).getEnv();

    auto final_pokemonA_v = final_env_v.getTeam(0).getPKV();
    auto final_pokemonB_v = final_env_v.getTeam(1).getPKV();

    EXPECT_EQ(final_pokemonA_v.getHP(), 0);
    EXPECT_LT(final_pokemonB_v.getHP(), pokemonB_nv.getFV_base(FV_HITPOINTS));
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::unique_ptr<PkCU> engine_;
};


TEST_F(MoveTest, Explosion) {
  runTest("explosion");
}


TEST_F(MoveTest, SelfDestruct) {
  runTest("selfdestruct");
}
