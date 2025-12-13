#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/pkai.h"
#include "pokemonai/team_volatile.h"

class TauntBugTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
  EnvironmentNonvolatile environment_nv;
};

TEST_F(TauntBugTest, PreemptsStatusMoveSameTurn) {
  // Scenario:
  // Aerodactyl (Fast, uses Taunt) vs Shuckle (Slow, uses Toxic)
  // Aerodactyl moves first. Taunt should apply.
  // Shuckle attempts Toxic. It should fail because it is taunted in the same turn.

  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("aerodactyl"))
        .addMove(pokedex_->move("taunt"))
        .setIV(FV_SPEED, 31)
        .setEV(FV_SPEED, 252)
        .setNature(pokedex_->nature("jolly")) // Max speed
        .setLevel(100));

  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("shuckle"))
        .addMove(pokedex_->move("toxic"))
        .setIV(FV_SPEED, 0) // Min speed
        .setLevel(100));

  environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment_nv);

  // Action: P0 uses Taunt (Move 0), P1 uses Toxic (Move 0)
  auto result = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));

  auto final_env = result.at(0).getEnv();

  // 1. Shuckle should be taunted
  EXPECT_GT(final_env.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);

  // 2. Aerodactyl should NOT be poisoned (Toxic should have failed)
  EXPECT_EQ(final_env.getTeam(0).teammate(0).getStatusAilment(), AIL_NV_NONE);
}
