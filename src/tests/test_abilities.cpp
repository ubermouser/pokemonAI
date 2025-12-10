#include <gtest/gtest.h>

#include <memory>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"

class AbilitiesTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
};


TEST_F(AbilitiesTest, NaturalCure) {
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("starmie"))
        .setAbility(pokedex_->getAbilities().at("natural cure"))
        .addMove(pokedex_->getMoves().at("surf"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("chansey"))
        .addMove(pokedex_->getMoves().at("softboiled"))
        .setLevel(100));
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("jolteon"))
        .addMove(pokedex_->getMoves().at("thunder wave"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);
  auto env_v = engine_->initialState();

  auto turn1_twave = engine_->updateState(
    env_v, Action::wait(), Action::move(0));
  auto turn2_swap = engine_->updateState(
    turn1_twave.at(0), Action::swap(1), Action::wait());

  { // Turn 1: Jolteon uses Thunder Wave on Starmie
    ASSERT_EQ(turn1_twave.size(), 1);
    auto starmie_after_twave = turn1_twave.at(0).getEnv().getTeam(TEAM_A).getPKV();
    EXPECT_EQ(starmie_after_twave.getStatusAilment(), AIL_NV_PARALYSIS);
  }
  { // Turn 2: Starmie switches out
    ASSERT_EQ(turn2_swap.size(), 1);
    auto starmie_after_switch = turn2_swap.at(0).getEnv().getTeam(TEAM_A).teammate(0);
    EXPECT_EQ(starmie_after_switch.getStatusAilment(), AIL_NV_NONE);
  }
}