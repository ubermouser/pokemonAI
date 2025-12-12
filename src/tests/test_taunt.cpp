#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/pkai.h"
#include "pokemonai/team_volatile.h"

class TauntTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(true);

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("aerodactyl"))
          .addMove(pokedex_->getMoves().at("taunt"))
          .addMove(pokedex_->getMoves().at("aerial ace")) // damage
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("pikachu"))); // backup

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("shuckle"))
          .addMove(pokedex_->getMoves().at("toxic")) // status
          .addMove(pokedex_->getMoves().at("wrap")) // damage
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
  EnvironmentNonvolatile environment_nv;
};

TEST_F(TauntTest, AppiesEffect) {
  // Aerodactyl uses Taunt on Shuckle
  auto taunt_result = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto env = taunt_result.at(0).getEnv();

  // Shuckle should have taunt duration
  EXPECT_GT(env.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);
  // Aerodactyl should not
  EXPECT_EQ(env.getTeam(0).teammate(0).status().cTeammate.taunt_duration, 0);
}

TEST_F(TauntTest, PreventsStatusMoves) {
  // Aerodactyl uses Taunt on Shuckle
  auto taunt_result = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto env = taunt_result.at(0).getEnv();

  // Shuckle tries to use Toxic (Move 0, Status) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(taunt_result.at(0), Action::move(0), TEAM_B));

  // Shuckle tries to use Constrict (Move 1, Physical) - Should be valid
  EXPECT_TRUE(engine_->isValidAction(taunt_result.at(0), Action::move(1), TEAM_B));
}

TEST_F(TauntTest, WearsOff) {
  // Turn 1: Aerodactyl uses Taunt. Shuckle is taunted (duration 3).
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto env1 = turn1.at(0).getEnv();
  EXPECT_EQ(env1.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 3);

  // Turn 2: Aerodactyl waits. Shuckle uses Constrict (valid). At end of turn/beginning of next, duration decrements.
  // Wait, taunt decrement is registered as PLUGIN_ON_BEGINNINGOFTURN.
  // So when we generate the *next* state, it runs.

  auto turn2 = engine_->updateState(turn1.at(0), Action::wait(), Action::move(1));
  auto env2 = turn2.at(0).getEnv();
  // Duration should be 2 now.
  EXPECT_EQ(env2.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 2);

  // Turn 3:
  auto turn3 = engine_->updateState(turn2.at(0), Action::wait(), Action::move(1));
  auto env3 = turn3.at(0).getEnv();
  EXPECT_EQ(env3.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 1);

  // Turn 4:
  auto turn4 = engine_->updateState(turn3.at(0), Action::wait(), Action::move(1));
  auto env4 = turn4.at(0).getEnv();
  EXPECT_EQ(env4.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);

  // Now Shuckle can use Toxic again.
  EXPECT_TRUE(engine_->isValidAction(turn4.at(0), Action::move(0), TEAM_B));
}
