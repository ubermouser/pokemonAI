#include "engine_test.hpp"

class RampageTest : public Gen4EngineTest {};

TEST_F(RampageTest, Outrage) {
  auto team_1 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("flygon"))
        .addMove(pokedex_->move("outrage"))
        .addMove(pokedex_->move("roost"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("pikachu")));
  auto team_2 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("metagross"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team_1, team_2, true);
  engine_->setEnvironment(environment);

  auto outrage_0 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto outrage_1 =
      engine_->updateState(outrage_0.where1(), Action::move(0), Action::wait());
  auto outrage_2 =
      engine_->updateState(outrage_1.where1(), Action::move(0), Action::wait());

  // the pokemon cannot switch out or perform other moves when outraging:
  EXPECT_TRUE(
      engine_->isValidAction(outrage_0.where1(), Action::move(0), TEAM_A));
  EXPECT_FALSE(
      engine_->isValidAction(outrage_0.where1(), Action::move(1), TEAM_A));
  EXPECT_FALSE(
      engine_->isValidAction(outrage_0.where1(), Action::swap(1), TEAM_A));

  // the pokemon is confused after outraging:
  EXPECT_EQ(
      outrage_2.where1().teammate(0, 0).status().cTeammate.confused,
      AIL_V_CONFUSED_5T);
  // the pokemon may switch out or perform other moves:
  EXPECT_TRUE(
      engine_->isValidAction(outrage_2.where1(), Action::move(0), TEAM_A));
  EXPECT_TRUE(
      engine_->isValidAction(outrage_2.where1(), Action::move(1), TEAM_A));
  EXPECT_TRUE(
      engine_->isValidAction(outrage_2.where1(), Action::swap(1), TEAM_A));
}

TEST_F(RampageTest, PetalDance) {
  auto team_1 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("venusaur"))
        .addMove(pokedex_->move("petal dance"))
        .addMove(pokedex_->move("tackle"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("pikachu")));
  auto team_2 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("metagross"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team_1, team_2, true);
  engine_->setEnvironment(environment);

  auto petal_0 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto petal_1 =
      engine_->updateState(petal_0.where1(), Action::move(0), Action::wait());
  auto petal_2 =
      engine_->updateState(petal_1.where1(), Action::move(0), Action::wait());

  // the pokemon cannot switch out or perform other moves when petal dancing:
  EXPECT_TRUE(
      engine_->isValidAction(petal_0.where1(), Action::move(0), TEAM_A));
  EXPECT_FALSE(
      engine_->isValidAction(petal_0.where1(), Action::move(1), TEAM_A));
  EXPECT_FALSE(
      engine_->isValidAction(petal_0.where1(), Action::swap(1), TEAM_A));

  // the pokemon is confused after petal dancing:
  EXPECT_EQ(
      petal_2.where1().teammate(0, 0).status().cTeammate.confused,
      AIL_V_CONFUSED_5T);
}

TEST_F(RampageTest, Thrash) {
  auto team_1 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("tauros"))
        .addMove(pokedex_->move("thrash"))
        .addMove(pokedex_->move("tackle"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("pikachu")));
  auto team_2 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("metagross"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team_1, team_2, true);
  engine_->setEnvironment(environment);

  auto thrash_0 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto thrash_1 =
      engine_->updateState(thrash_0.where1(), Action::move(0), Action::wait());
  auto thrash_2 =
      engine_->updateState(thrash_1.where1(), Action::move(0), Action::wait());

  // the pokemon cannot switch out or perform other moves when thrashing:
  EXPECT_TRUE(
      engine_->isValidAction(thrash_0.where1(), Action::move(0), TEAM_A));
  EXPECT_FALSE(
      engine_->isValidAction(thrash_0.where1(), Action::move(1), TEAM_A));
  EXPECT_FALSE(
      engine_->isValidAction(thrash_0.where1(), Action::swap(1), TEAM_A));

  // the pokemon is confused after thrashing:
  EXPECT_EQ(
      thrash_2.where1().teammate(0, 0).status().cTeammate.confused,
      AIL_V_CONFUSED_5T);
}
