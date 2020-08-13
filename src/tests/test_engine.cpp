#include <gtest/gtest.h>

#include <memory>

#include <inc/engine.h>
#include <inc/pokedex_static.h>
#include <inc/pkCU.h>


class EngineTest : public ::testing::Test {
protected:
  void SetUp() override {
    verbose = 4;
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>(SIZE_MAX, true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
};


TEST_F(EngineTest, PrimaryHitAndCrit) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("charmander"))
        .addMove(pokedex_->getMoves().at("cut")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), AT_MOVE_0, AT_MOVE_NOTHING);

  result.printStates(environment);
  EXPECT_EQ(result.size(), 3);
}


TEST_F(EngineTest, PainSplit) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("dusknoir"))
        .addMove(pokedex_->getMoves().at("pain split"))
        .addMove(pokedex_->getMoves().at("shadow sneak"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto split_pain = engine_->updateState(engine_->initialState(), AT_MOVE_0, AT_MOVE_1);

  split_pain.printStates(environment);
  auto& result = split_pain.at(0).getEnv();
  EXPECT_EQ(result.getTeam(0).teammate(0).getHP(), result.getTeam(1).teammate(0).getHP());
  EXPECT_EQ(result.getTeam(0).teammate(0).getMV(0).getPP(), 31);
}


TEST_F(EngineTest, Aromatherapy) {
  const auto& pokemons = pokedex_->getPokemon();
  const auto& moves = pokedex_->getMoves();
  auto agentTeam = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokemons.at("pikachu")))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokemons.at("blissey"))
        .addMove(moves.at("aromatherapy")));
  auto otherTeam = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokemons.at("crobat"))
        .addMove(moves.at("toxic")));
  auto environment = EnvironmentNonvolatile(agentTeam, otherTeam, true);
  engine_->setEnvironment(environment);

  auto pikachu_poisoned = engine_->updateState(engine_->initialState(), AT_MOVE_NOTHING, AT_MOVE_0);
  auto blissey_poisoned = engine_->updateState(pikachu_poisoned.at(0).getEnv(), AT_SWITCH_1, AT_MOVE_0);
  auto all_cured = engine_->updateState(blissey_poisoned.at(0).getEnv(), AT_MOVE_0, AT_MOVE_NOTHING);

  EXPECT_EQ(pikachu_poisoned.at(0).getEnv().getTeam(0).teammate(0).getStatusAilment(), AIL_NV_POISON_TOXIC);
  EXPECT_EQ(blissey_poisoned.at(0).getEnv().getTeam(0).teammate(1).getStatusAilment(), AIL_NV_POISON_TOXIC);
  // aromatherapy used once:
  EXPECT_EQ(all_cured.at(0).getEnv().getTeam(0).teammate(1).getMV(0).getPP(), 7);
  // both pikachu and blissey cured:
  EXPECT_EQ(all_cured.at(0).getEnv().getTeam(0).teammate(0).getStatusAilment(), AIL_NV_NONE);
  EXPECT_EQ(all_cured.at(0).getEnv().getTeam(0).teammate(1).getStatusAilment(), AIL_NV_NONE);
}
