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


TEST_F(MoveTest, Outrage) {
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

  // TODO(@drendleman) the pokemon's outrage counter doesn't decrease when the enemy is dead:
}


TEST_F(MoveTest, PetalDance) {
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


TEST_F(MoveTest, Thrash) {
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
