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


class SuckerPunchTest : public MoveTest {
protected:
  void SetUp() override {
    MoveTest::SetUp();

    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("spiritomb"))
          .addMove(pokedex_->getMoves().at("sucker punch"))
          .addMove(pokedex_->getMoves().at("will-o-wisp"))
          .addMove(pokedex_->getMoves().at("shock wave"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("azelf"))
          .setLevel(100));
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);
  }
};

TEST_F(SuckerPunchTest, no_damage_against_status_effect_move) {
  auto suckerpunch_status = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(1));

  EXPECT_EQ(suckerpunch_status.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 1.);
  EXPECT_EQ(suckerpunch_status.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
}

TEST_F(SuckerPunchTest, no_damage_against_swapping_pokemon) {
  auto suckerpunch_move = engine_->updateState(engine_->initialState(), Action::move(0), Action::swap(1));

  EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 1.);
  EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(1).teammate(1).getPercentHP(), 1.);
  EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
}

TEST_F(SuckerPunchTest, full_damage_against_attacking_pokemon) {
  auto suckerpunch_dmg = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(2));

  EXPECT_LT(suckerpunch_dmg.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 0.75);
  EXPECT_EQ(suckerpunch_dmg.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
}


TEST_F(MoveTest, PainSplit) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("dusknoir"))
        .addMove(pokedex_->getMoves().at("pain split"))
        .addMove(pokedex_->getMoves().at("shadow sneak"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto split_pain = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(1));

  auto result = split_pain.at(0).getEnv();
  EXPECT_EQ(result.getTeam(0).teammate(0).getHP(), result.getTeam(1).teammate(0).getHP());
  EXPECT_EQ(result.getTeam(0).teammate(0).getMV(0).getPP(), 31);
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
  auto blissey_poisoned = engine_->updateState(pikachu_poisoned.at(0), Action::swap(1), Action::move(0));
  auto all_cured = engine_->updateState(blissey_poisoned.at(0), Action::move(0), Action::wait());

  EXPECT_EQ(pikachu_poisoned.at(0).getEnv().getTeam(0).teammate(0).getStatusAilment(), AIL_NV_POISON_TOXIC);
  EXPECT_EQ(blissey_poisoned.at(0).getEnv().getTeam(0).teammate(1).getStatusAilment(), AIL_NV_POISON_TOXIC);
  // aromatherapy used once:
  EXPECT_EQ(all_cured.at(0).getEnv().getTeam(0).teammate(1).getMV(0).getPP(), 7);
  // both pikachu and blissey cured:
  EXPECT_EQ(all_cured.at(0).getEnv().getTeam(0).teammate(0).getStatusAilment(), AIL_NV_NONE);
  EXPECT_EQ(all_cured.at(0).getEnv().getTeam(0).teammate(1).getStatusAilment(), AIL_NV_NONE);
}


TEST_F(MoveTest, Heal50) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("pidgey"))
        .addMove(pokedex_->getMoves().at("wing attack"))
        .addMove(pokedex_->getMoves().at("roost"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto pidgey_tackled = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  auto pidgey_roosted = engine_->updateState(pidgey_tackled.at(1), Action::move(1), Action::wait());

  EXPECT_EQ(pidgey_tackled.at(1).getEnv().getTeam(0).teammate(0).getHP(), 30);
  EXPECT_EQ(pidgey_roosted.at(0).getEnv().getTeam(0).teammate(0).getHP(), 125);
  EXPECT_EQ(pidgey_roosted.at(0).getEnv().getTeam(0).teammate(0).getMV(1).getPP(), 15);
}


TEST_F(MoveTest, HiddenPower) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("heatran"))
        .addMove(pokedex_->getMoves().at("hidden power"))
        .setLevel(100))
        .initialize();
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto hidden_power = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  // rock_t with 30 power
  EXPECT_EQ(hidden_power.at(0).getEnv().getTeam(1).getPKV().getHP(), 233);
}


TEST_F(MoveTest, Pursuit) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("scizor"))
        .addMove(pokedex_->getMoves().at("pursuit"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("azelf")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto pursuit_switch = engine_->updateState(engine_->initialState(), Action::move(0), Action::swap(1));
  auto pursuit_noswitch = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  // the pokemon switching out receives 2x the damage as a pokemon that doesn't:
  EXPECT_LT(pursuit_switch.at(0).getEnv().getTeam(1).teammate(0).getHP(),
            pursuit_noswitch.at(0).getEnv().getTeam(1).teammate(0).getHP());
  // no damage to the pokemon who switched in:
  EXPECT_EQ(pursuit_switch.at(0).getEnv().getTeam(1).teammate(1).getPercentHP(), 1.);
}


TEST_F(MoveTest, Outrage) {
  auto team_1 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("flygon"))
        .addMove(pokedex_->getMoves().at("outrage"))
        .addMove(pokedex_->getMoves().at("roost"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("pikachu")));
  auto team_2 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("metagross"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team_1, team_2, true);
  engine_->setEnvironment(environment);

  auto outrage_0 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto outrage_1 = engine_->updateState(outrage_0.at(0), Action::move(0), Action::wait());
  auto outrage_2 = engine_->updateState(outrage_1.at(0), Action::move(0), Action::wait());

  // the pokemon cannot switch out or perform other moves when outraging:
  EXPECT_TRUE(engine_->isValidAction(outrage_0.at(0), Action::move(0), TEAM_A));
  EXPECT_FALSE(engine_->isValidAction(outrage_0.at(0), Action::move(1), TEAM_A));
  EXPECT_FALSE(engine_->isValidAction(outrage_0.at(0), Action::swap(1), TEAM_A));

  // the pokemon is confused after outraging:
  EXPECT_EQ(
      outrage_2.at(0).getEnv().getTeam(0).teammate(0).status().cTeammate.confused, AIL_V_CONFUSED_5T);
  // the pokemon may switch out or perform other moves:
  EXPECT_TRUE(engine_->isValidAction(outrage_2.at(0), Action::move(0), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(outrage_2.at(0), Action::move(1), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(outrage_2.at(0), Action::swap(1), TEAM_A));

  // TODO(@drendleman) the pokemon's outrage counter doesn't decrease when the enemy is dead:
}


class UTurnTest : public MoveTest {
protected:
  void SetUp() override {
    MoveTest::SetUp();

    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("scizor"))
          .addMove(pokedex_->getMoves().at("u-turn"))
          .setInitialItem(pokedex_->getItems().at("life orb"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("torterra"))
          .addMove(pokedex_->getMoves().at("stealth rock"))
          .setLevel(50));
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);

    setup_swap = engine_->updateState(engine_->initialState(), Action::wait(), Action::swap(1));
    setup_sr = engine_->updateState(setup_swap.at(0), Action::wait(), Action::move(0));
    uturn_to_ally = engine_->updateState(
      setup_swap.at(0), Action::moveAlly(0, 1), Action::wait());
    uturn_to_ally_with_sr = engine_->updateState(
      setup_sr.at(0), Action::moveAlly(0, 1), Action::wait());
    swap_to_scyzor = engine_->updateState(
      uturn_to_ally.at(0), Action::wait(), Action::swap(0));
    uturn_no_ally = engine_->updateState(
      swap_to_scyzor.at(0), Action::wait(), Action::moveAlly(0, 0));
  }

  PossibleEnvironments setup_swap;
  PossibleEnvironments setup_sr;
  PossibleEnvironments uturn_to_ally;
  PossibleEnvironments uturn_to_ally_with_sr;
  PossibleEnvironments swap_to_scyzor;
  PossibleEnvironments uturn_no_ally;
};


TEST_F(UTurnTest, requires_pokemon_to_swap_if_ally_exists) {
  EXPECT_FALSE(engine_->isValidAction(setup_sr.at(0), Action::move(0), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(setup_sr.at(0), Action::moveAlly(0, 1), TEAM_A));
  EXPECT_FALSE(engine_->isValidAction(setup_sr.at(0), Action::moveAlly(0, 0), TEAM_A));
}


TEST_F(UTurnTest, can_still_be_used_without_swap_if_no_allies_exist) {
  EXPECT_FALSE(engine_->isValidAction(swap_to_scyzor.at(0), Action::move(0), TEAM_B));
  EXPECT_TRUE(engine_->isValidAction(swap_to_scyzor.at(0), Action::moveAlly(0, 0), TEAM_B));
  EXPECT_FALSE(engine_->isValidAction(swap_to_scyzor.at(0), Action::moveAlly(0, 1), TEAM_B));
}

TEST_F(UTurnTest, damages_enemy_and_swaps_to_ally) {
  // pp decremented
  EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 31);
  // item effect (life orb) applies
  EXPECT_NEAR(uturn_to_ally.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 0.9, 0.005);
  // ally has swapped out
  EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(0).getICPKV(), 1);
  EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(1).teammate(1).getPercentHP(), 0.); // enemy weakling deleted
}


TEST_F(UTurnTest, DISABLED_damages_enemy_and_swaps_to_ally_with_stealth_rock) {
  // life orb applies to attacking teammate
  EXPECT_NEAR(uturn_to_ally_with_sr.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 0.9, 0.005);
  // stealth-rock applies to entering teammate
  EXPECT_NEAR(uturn_to_ally_with_sr.at(0).getEnv().getTeam(0).teammate(1).getPercentHP(), 0.875, 0.005);
}


TEST_F(UTurnTest, damages_enemy_but_doesnt_swap_if_no_allies_exist) {
  // pp decremented
  EXPECT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).teammate(0).getMV(0).getPP(), 31);
  // item effect (life orb) applies
  EXPECT_FLOAT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 0.9);
  // ally NOT swapped out
  EXPECT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).getICPKV(), 0);
}

class PaybackTest : public MoveTest {
protected:
  void SetUp() override {
    MoveTest::SetUp();

    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("machamp"))
          .addMove(pokedex_->getMoves().at("payback"))
          .addMove(pokedex_->getMoves().at("bullet punch"))
          .setIV(FV_SPEED, 30) // always faster
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->getPokemon().at("machoke"))
          .addMove(pokedex_->getMoves().at("leer"))
          .addMove(pokedex_->getMoves().at("bullet punch"))
          .setIV(FV_DEFENSE, 20) // match Machamp's base defense
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);

    setup_payback = engine_->updateState(engine_->initialState(), Action::wait(), Action::swap(1));

    // we need this to be true so the moving first and enemy switching tests
    // return the same damage taken
    EXPECT_EQ(
      environment_nv.getTeam(0).teammate(0).getFV_base(FV_DEFENSE),
      environment_nv.getTeam(0).teammate(1).getFV_base(FV_DEFENSE));
  }

  EnvironmentNonvolatile environment_nv;
  PossibleEnvironments setup_payback;
};

TEST_F(PaybackTest, normal_power_when_moving_first) {
  auto payback_normal_power = engine_->updateState(
    setup_payback.at(0), Action::move(0), Action::move(0));

  EXPECT_EQ(payback_normal_power.at(0).getEnv().getTeam(1).getPKV().getMissingHP(), 31);
}

TEST_F(PaybackTest, normal_power_when_enemy_switching) {
  // NOTE: normal damage on switching is technically gen-V+ behavior, but
  // we test it here as smogon uses this behavior for Gen-IV
  auto payback_normal_power = engine_->updateState(
    setup_payback.at(0), Action::move(0), Action::swap(0));

  EXPECT_EQ(payback_normal_power.at(0).getEnv().getTeam(1).getPKV().getMissingHP(), 31);
}

TEST_F(PaybackTest, double_power_when_moving_second) {
  // machoke uses a priority move, machamp will move second
  auto payback_doubled_power = engine_->updateState(
    setup_payback.at(0), Action::move(0), Action::move(1));

  EXPECT_EQ(payback_doubled_power.at(0).getEnv().getTeam(1).getPKV().getMissingHP(), 63);
}
