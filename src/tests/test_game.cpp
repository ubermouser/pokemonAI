#include <gtest/gtest.h>

#include <memory>

#include <inc/engine.h>
#include <inc/game.h>
#include <inc/pokedex_static.h>
#include <inc/pkCU.h>


class GameTest : public ::testing::Test {
protected:
  void SetUp() override {
    verbose = 4;
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>(SIZE_MAX, true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
};


TEST_F(GameTest, RolloutPokemon) {
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("charmander"))
        .addMove(pokedex_->getMoves().at("cut"))
        .setLevel(100));
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("bulbasaur"))
        .addMove(pokedex_->getMoves().at("tackle"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team_a, team_b);
  auto game = Game()
      .setMaxMatches(11)
      .setVerbosity(3)
      .setEnvironment(environment);

  HeatResult result = game.rollout();

  EXPECT_GE(result.matchesPlayed, 6);
}