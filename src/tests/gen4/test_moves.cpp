#include "engine_test.hpp"


class MoveTest : public Gen4EngineTest {};


TEST_F(MoveTest, PainSplit) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("dusknoir"))
        .addMove(pokedex_->move("pain split"))
        .addMove(pokedex_->move("shadow sneak"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto split_pain = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(1));

  auto result = split_pain.where1().getEnv();
  EXPECT_EQ(result.teammate(0, 0).getHP(), result.teammate(1, 0).getHP());
  EXPECT_EQ(result.teammate(0, 0).getMV(0).getPP(), 31);
}


TEST_F(MoveTest, Aromatherapy) {
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

  auto pikachu_poisoned = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  auto blissey_poisoned = engine_->updateState(
      pikachu_poisoned.where1(), Action::swap(1), Action::move(0));
  auto all_cured = engine_->updateState(
      blissey_poisoned.where1(), Action::move(0), Action::wait());

  EXPECT_EQ(
      pikachu_poisoned.where1().teammate(0, 0).getStatusAilment(),
      AIL_NV_POISON_TOXIC);
  EXPECT_EQ(
      blissey_poisoned.where1().teammate(0, 1).getStatusAilment(),
      AIL_NV_POISON_TOXIC);
  // aromatherapy used once:
  EXPECT_EQ(all_cured.where1().teammate(0, 1).getMV(0).getPP(), 7);
  // both pikachu and blissey cured:
  EXPECT_EQ(all_cured.where1().teammate(0, 0).getStatusAilment(), AIL_NV_NONE);
  EXPECT_EQ(all_cured.where1().teammate(0, 1).getStatusAilment(), AIL_NV_NONE);
}


TEST_F(MoveTest, Heal50) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("pidgey"))
        .addMove(pokedex_->move("wing attack"))
        .addMove(pokedex_->move("roost"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto pidgey_tackled = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  // Select state where enemy critically hit for significant damage
  auto pidgey_crit = pidgey_tackled.where1Crit(1);
  auto pidgey_roosted =
      engine_->updateState(pidgey_crit, Action::move(1), Action::wait());

  EXPECT_EQ(pidgey_crit.teammate(0, 0).getHP(), 30);
  EXPECT_EQ(pidgey_roosted.where1Hit(0).teammate(0, 0).getHP(), 125);
  EXPECT_EQ(pidgey_roosted.where1Hit(0).teammate(0, 0).getMV(1).getPP(), 15);
}


TEST_F(MoveTest, HiddenPower) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("heatran"))
        .addMove(pokedex_->move("hidden power"))
        .setLevel(100))
        .initialize();
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto hidden_power = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  // rock_t with 30 power
  EXPECT_EQ(hidden_power.where1().teammate(1, 0).getHP(), 233);
}


TEST_F(MoveTest, Pursuit) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("scizor"))
        .addMove(pokedex_->move("pursuit"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("azelf")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto pursuit_switch = engine_->updateState(engine_->initialState(), Action::move(0), Action::swap(1));
  auto pursuit_noswitch = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  // the pokemon switching out receives 2x the damage as a pokemon that doesn't:
  EXPECT_LT(
      pursuit_switch.where1Hit(0).teammate(1, 0).getHP(),
      pursuit_noswitch.where1Hit(0).teammate(1, 0).getHP());
  // no damage to the pokemon who switched in:
  EXPECT_EQ(pursuit_switch.where1Hit(0).teammate(1, 1).getPercentHP(), 1.);
}
