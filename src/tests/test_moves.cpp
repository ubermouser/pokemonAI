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
  EnvironmentNonvolatile environment_nv;
};

class SuicideMoveTest : public MoveTest {
protected:
  void SetUp() override {
    MoveTest::SetUp();

    auto team_a_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("electrode"))
          .addMove(pokedex_->move("explosion"))
          .addMove(pokedex_->move("selfdestruct"))
          .setLevel(100));
    auto team_b_nv = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu"))
          .addMove(pokedex_->move("volt tackle"))
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team_a_nv, team_b_nv, true);
    engine_->setEnvironment(environment_nv);
  }
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
          .setBase(pokedex_->pokemon("spiritomb"))
          .addMove(pokedex_->move("sucker punch"))
          .addMove(pokedex_->move("will-o-wisp"))
          .addMove(pokedex_->move("shock wave"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("azelf"))
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);
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
        .setBase(pokedex_->pokemon("dusknoir"))
        .addMove(pokedex_->move("pain split"))
        .addMove(pokedex_->move("shadow sneak"))
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
        .setBase(pokedex_->pokemon("pidgey"))
        .addMove(pokedex_->move("wing attack"))
        .addMove(pokedex_->move("roost"))
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
        .setBase(pokedex_->pokemon("heatran"))
        .addMove(pokedex_->move("hidden power"))
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
  EXPECT_LT(pursuit_switch.at(0).getEnv().getTeam(1).teammate(0).getHP(),
            pursuit_noswitch.at(0).getEnv().getTeam(1).teammate(0).getHP());
  // no damage to the pokemon who switched in:
  EXPECT_EQ(pursuit_switch.at(0).getEnv().getTeam(1).teammate(1).getPercentHP(), 1.);
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
          .setBase(pokedex_->pokemon("scizor"))
          .addMove(pokedex_->move("u-turn"))
          .setInitialItem(pokedex_->item("life orb"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("torterra"))
          .addMove(pokedex_->move("stealth rock"))
          .setLevel(50));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);

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


TEST_F(UTurnTest, damages_enemy_and_swaps_to_ally_with_stealth_rock) {
  // life orb applies to attacking teammate
  EXPECT_NEAR(uturn_to_ally_with_sr.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 0.9, 0.005);
  // stealth-rock applies to entering teammate
  EXPECT_NEAR(uturn_to_ally_with_sr.at(0).getEnv().getTeam(0).teammate(1).getPercentHP(), 0.9375, 0.005);
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
          .setBase(pokedex_->pokemon("machamp"))
          .addMove(pokedex_->move("payback"))
          .addMove(pokedex_->move("bullet punch"))
          .setIV(FV_SPEED, 30) // always faster
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("machoke"))
          .addMove(pokedex_->move("leer"))
          .addMove(pokedex_->move("bullet punch"))
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


class TrickTest : public MoveTest {
protected:
  void SetUp() override {
    MoveTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("kadabra"))
          .addMove(pokedex_->move("trick"))
          .setInitialItem(pokedex_->item("choice specs"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("alakazam"))
          .addMove(pokedex_->move("trick"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("softboiled"))
          .addMove(pokedex_->move("charm"))
          .setInitialItem(pokedex_->item("leftovers"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gastrodon"))
          .setAbility(pokedex_->ability("sticky hold"))
          .setInitialItem(pokedex_->item("choice band"))
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};


TEST_F(TrickTest, item_for_item) {
  auto trick_item = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::wait());

  auto final_env_v = trick_item.at(0).getEnv();
  EXPECT_EQ(final_env_v.getTeam(0).getPKV().getItem().getName(), "leftovers");
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "choice specs");
}


TEST_F(TrickTest, item_behavior_propagates) {
  auto trick_item = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::move(0));

  auto final_env_v = trick_item.at(0).getEnv();
  EXPECT_FALSE(engine_->isValidAction(final_env_v, Action::move(1), 1));
}


TEST_F(TrickTest, no_item_for_item) {
  auto trick_no_item = engine_->updateState(
    engine_->initialState(), Action::swap(1), Action::wait());
  auto final_trick = engine_->updateState(
    trick_no_item.at(0), Action::move(0), Action::wait());

  auto final_env_v = final_trick.at(0).getEnv();
  EXPECT_EQ(final_env_v.getTeam(0).getPKV().getItem().getName(), "leftovers");
  EXPECT_FALSE(final_env_v.getTeam(1).getPKV().hasItem());
}


TEST_F(TrickTest, item_for_no_item) {
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("blissey"))
        .addMove(pokedex_->move("softboiled"))
        .setLevel(100));
  engine_->setEnvironment(EnvironmentNonvolatile(environment_nv.getTeam(0), team_b, true));

  auto trick_item = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::wait());

  auto final_env_v = trick_item.at(0).getEnv();
  EXPECT_FALSE(final_env_v.getTeam(0).getPKV().hasItem());
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "choice specs");
}


TEST_F(TrickTest, sticky_hold_fails) {
  auto trick_item = engine_->updateState(
    engine_->initialState(), Action::move(0), Action::swap(1));

  auto final_env_v = trick_item.at(0).getEnv();
  EXPECT_EQ(final_env_v.getTeam(0).getPKV().getItem().getName(), "choice specs");
  EXPECT_EQ(final_env_v.getTeam(1).getPKV().getItem().getName(), "choice band");
}


class TauntTest : public MoveTest {
protected:
  void SetUp() override {
    MoveTest::SetUp();
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("steelix"))
          .addMove(pokedex_->move("taunt"))
          .addMove(pokedex_->move("strength")) // damage
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu"))); // backup

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("shuckle"))
          .addMove(pokedex_->move("toxic")) // status
          .addMove(pokedex_->move("strength")) // damage
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(TauntTest, AppliesEffect) {
  // Steelix uses Taunt on Shuckle
  auto taunt_result = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto env = taunt_result.at(0).getEnv();

  // Shuckle should have taunt duration
  EXPECT_GT(env.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);
  // Steelix should not
  EXPECT_EQ(env.getTeam(0).teammate(0).status().cTeammate.taunt_duration, 0);
}

TEST_F(TauntTest, PreventsStatusMoves) {
  // Steelix uses Taunt on Shuckle
  auto taunt_result = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto env = taunt_result.at(0).getEnv();

  // Shuckle tries to use Toxic (Move 0, Status) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(taunt_result.at(0), Action::move(0), TEAM_B));

  // Shuckle tries to use Constrict (Move 1, Physical) - Should be valid
  EXPECT_TRUE(engine_->isValidAction(taunt_result.at(0), Action::move(1), TEAM_B));
}

TEST_F(TauntTest, WearsOff) {
  // Turn 1: Steelix uses Taunt. Shuckle is taunted (duration 3-5).
  // Note: PkCU branches state. We pick the first environment and follow it.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  turn1.printStates();
  // Verify we have multiple outcomes (3, 4, 5 turns)
  // We expect at least 3 states due to triplicateState, possibly more if other RNG happened (but unlikely here)
  EXPECT_GE(turn1.size(), 3);

  // Pick one state to follow.
  auto initial_env = turn1.at(0);
  auto initial_duration = initial_env.getEnv().getTeam(1).teammate(0).status().cTeammate.taunt_duration;

  EXPECT_GE(initial_duration, 3);
  EXPECT_LE(initial_duration, 5);

  auto next_turn = turn1;
  auto current_state = turn1.at(0);
  for (uint32_t i = 0; i < initial_duration; ++i) {
      // Duration should decrement each turn
      next_turn = engine_->updateState(current_state, Action::move(1), Action::move(1));
      current_state = next_turn.at(0);
      next_turn.printStates();

      auto current_duration = current_state.getEnv().getTeam(1).teammate(0).status().cTeammate.taunt_duration;
      EXPECT_EQ(current_duration, initial_duration - 1 - i);
  }

  // After duration expires, duration should be 0
  EXPECT_EQ(current_state.getEnv().getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);

  // Now Shuckle can use Toxic again.
  EXPECT_TRUE(engine_->isValidAction(current_state, Action::move(0), TEAM_B));
}


TEST_F(TauntTest, PreemptsStatusMoveSameTurn) {
  // Scenario:
  // Steelix (Fast, uses Taunt) vs Shuckle (Slow, uses Toxic)
  // Steelix moves first. Taunt should apply.
  // Shuckle attempts Toxic. It should fail because it is taunted in the same turn.
  engine_->setEnvironment(environment_nv);

  // Action: P0 uses Taunt (Move 0), P1 uses Toxic (Move 0)
  auto result = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));

  auto final_env = result.at(0).getEnv();

  // 1. Shuckle should be taunted
  EXPECT_GT(final_env.getTeam(1).teammate(0).status().cTeammate.taunt_duration, 0);

  // 2. Steelix should NOT be poisoned (Toxic should have failed)
  EXPECT_EQ(final_env.getTeam(0).teammate(0).getStatusAilment(), AIL_NV_NONE);
}

class ScreenTest : public MoveTest {
protected:
  void SetUp() override {
    MoveTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("reflect"))
          .addMove(pokedex_->move("light screen"))
          .addMove(pokedex_->move("strength")) // Physical
          .addMove(pokedex_->move("swift")) // Special
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mewtwo"))
          .addMove(pokedex_->move("strength")) // Physical
          .addMove(pokedex_->move("swift")) // Special
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(ScreenTest, ReflectReducesPhysicalDamage) {
  auto baseline_result = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
  fpType mew_health_baseline = baseline_result.at(0).getEnv().getTeam(0).teammate(0).getPercentHP();

  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto turn1_env = turn1.at(0).getEnv();

  EXPECT_EQ(turn1_env.getTeam(0).getNonVolatile().reflect, 5);

  auto turn2 = engine_->updateState(turn1.at(0), Action::wait(), Action::move(0));
  fpType mew_health_reflect = turn2.at(0).getEnv().getTeam(0).teammate(0).getPercentHP();

  fpType damage_baseline = 1.0 - mew_health_baseline;
  fpType damage_reflect = 1.0 - mew_health_reflect;

  EXPECT_LT(damage_reflect, damage_baseline);
  EXPECT_NEAR(damage_reflect, damage_baseline * 0.5, 0.01);
}

TEST_F(ScreenTest, LightScreenReducesSpecialDamage) {
  auto baseline_result = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(1));
  fpType mew_health_baseline = baseline_result.at(0).getEnv().getTeam(0).teammate(0).getPercentHP();

  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(1), Action::wait());
  auto turn1_env = turn1.at(0).getEnv();

  EXPECT_EQ(turn1_env.getTeam(0).getNonVolatile().lightScreen, 5);

  auto turn2 = engine_->updateState(turn1.at(0), Action::wait(), Action::move(1));
  fpType mew_health_screen = turn2.at(0).getEnv().getTeam(0).teammate(0).getPercentHP();

  fpType damage_baseline = 1.0 - mew_health_baseline;
  fpType damage_screen = 1.0 - mew_health_screen;

  EXPECT_LT(damage_screen, damage_baseline);
  EXPECT_NEAR(damage_screen, damage_baseline * 0.5, 0.01);
}

TEST_F(ScreenTest, ReflectDurationDecrements) {
  // Turn 1: Mew uses Reflect
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto current_state = turn1.at(0);
  EXPECT_EQ(current_state.getEnv().getTeam(0).getNonVolatile().reflect, 5);

  // Turns 2-5: Reflect is active and decrements
  for (int i = 0; i < 4; ++i) {
      // Advance turn using a move instead of wait, to ensure beginning of turn hooks run
      auto result = engine_->updateState(current_state, Action::move(2), Action::move(0));
      current_state = result.at(0);
      int expected_duration = 4 - i;
      EXPECT_EQ(current_state.getEnv().getTeam(0).getNonVolatile().reflect, expected_duration) << "Turn " << (i+2);
  }

  EXPECT_EQ(current_state.getEnv().getTeam(0).getNonVolatile().reflect, 1);

  // Turn 6: Decrement to 0.
  auto turn6 = engine_->updateState(current_state, Action::move(2), Action::move(0));
  EXPECT_EQ(turn6.at(0).getEnv().getTeam(0).getNonVolatile().reflect, 0);
}

TEST_F(ScreenTest, LightScreenDurationDecrements) {
  // Turn 1: Mew uses Light Screen
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(1), Action::wait());
  auto current_state = turn1.at(0);
  EXPECT_EQ(current_state.getEnv().getTeam(0).getNonVolatile().lightScreen, 5);

  for (int i = 0; i < 4; ++i) {
      auto result = engine_->updateState(current_state, Action::move(2), Action::move(0));
      current_state = result.at(0);
  }

  EXPECT_EQ(current_state.getEnv().getTeam(0).getNonVolatile().lightScreen, 1);

  auto turn6 = engine_->updateState(current_state, Action::move(2), Action::move(0));
  EXPECT_EQ(turn6.at(0).getEnv().getTeam(0).getNonVolatile().lightScreen, 0);
}

TEST_F(ScreenTest, FailsIfAlreadyActive) {
  // Turn 1: Mew uses Reflect
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  EXPECT_EQ(turn1.at(0).getEnv().getTeam(0).getNonVolatile().reflect, 5);

  // Turn 2: Mew tries to use Reflect again.
  // Should decrement to 4, then fail to reset to 5.

  auto turn2 = engine_->updateState(turn1.at(0), Action::move(0), Action::wait());
  EXPECT_EQ(turn2.at(0).getEnv().getTeam(0).getNonVolatile().reflect, 4);
}
