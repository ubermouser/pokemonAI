#include <gtest/gtest.h>

#include <memory>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"


class EngineTest : public ::testing::Test {
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


TEST_F(EngineTest, PrimaryHitAndCrit) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("charmander"))
        .addMove(pokedex_->getMoves().at("cut")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  // pokemon may move freely when both are alive:
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Action::move(0), TEAM_A));
  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.at(0).hasHit(0), true);
  EXPECT_EQ(result.at(1).hasHit(0), false);
  EXPECT_EQ(result.at(2).hasCrit(0), true);
}


TEST_F(EngineTest, Swap) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("torkoal"))
        .addMove(pokedex_->getMoves().at("explosion")))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("squirtle"))
        .addMove(pokedex_->getMoves().at("surf")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto swap_squirtle = engine_->updateState(engine_->initialState(), Action::swap(1), Action::wait());
  auto torkoal_dead = engine_->updateState(swap_squirtle.at(0), Action::move(0), Action::wait());
  auto both_dead = engine_->updateState(swap_squirtle.at(0), Action::wait(), Action::move(0));
  
  // active pokemon has changed:
  EXPECT_EQ(engine_->initialState().getTeam(0).getICPKV(), 0);
  EXPECT_EQ(swap_squirtle.at(0).getEnv().getTeam(0).getICPKV(), 1);

  // pokemon may not swap to themselves:
  EXPECT_FALSE(engine_->isValidAction(engine_->initialState(), Action::swap(0), TEAM_A));
  EXPECT_FALSE(engine_->isValidAction(swap_squirtle.at(0), Action::swap(1), TEAM_A));
  // pokemon may swap when alive:
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_B));
  // dead pokemon may swap:
  EXPECT_TRUE(engine_->isValidAction(torkoal_dead.at(0), Action::swap(1), TEAM_B));
  // living pokemon may NOT swap when the enemy is dead:
  EXPECT_FALSE(engine_->isValidAction(torkoal_dead.at(0), Action::swap(0), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(torkoal_dead.at(0), Action::wait(), TEAM_A));
  // if BOTH pokemon are dead, both pokemon may swap:
  EXPECT_TRUE(engine_->isValidAction(both_dead.at(0), Action::swap(0), TEAM_A));
  EXPECT_TRUE(engine_->isValidAction(both_dead.at(0), Action::swap(1), TEAM_B));
  // move counts should be accurate:
  EXPECT_EQ(engine_->getValidActions(torkoal_dead.at(0).getEnv(), TEAM_B).size(), 1);
}


TEST_F(EngineTest, InvalidAction) {
  engine_->setAllowInvalidMoves(false);

  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("charmander"))
        .addMove(pokedex_->getMoves().at("cut")))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("bulbasaur"))
        .addMove(pokedex_->getMoves().at("razor leaf")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto constInitialState = engine_->initialState();

  // Create a mutable copy of the state
  auto mutableStateData = constInitialState.data();
  // Faint the second pokemon
  TeamStatus teamStatus{};
  PokemonVolatile faintedPokemon(
    constInitialState.nv().getTeam(TEAM_A).teammate(1),
    mutableStateData.teams[TEAM_A].teammates[1],
    teamStatus
  );
  faintedPokemon.setHP(0);

  // Create a new ConstEnvironmentVolatile for the test
  ConstEnvironmentVolatile initialState(constInitialState.nv(), mutableStateData);

  // Pokemon may not swap to a dead pokemon
  EXPECT_EQ(engine_->isValidAction(initialState, Action::swap(1), TEAM_A).reason, IsValidResult::SWITCH_POKEMON_DEAD);

  try {
    engine_->updateState(initialState, Action::swap(1), Action::wait());
    FAIL() << "Expected std::runtime_error";
  } catch(std::runtime_error const & err) {
    EXPECT_EQ(err.what(), std::string("Invalid Action for Team A: Cannot switch to a dead pokemon"));
  } catch(...) {
    FAIL() << "Expected std::runtime_error";
  }
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

  auto split_pain = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(1));

  auto result = split_pain.at(0).getEnv();
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


TEST_F(EngineTest, Heal50) {
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


TEST_F(EngineTest, GroundConditions) {
  auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile() // has rapid spin
        .setBase(pokedex_->getPokemon().at("forretress"))
        .addMove(pokedex_->getMoves().at("stealth rock"))
        .addMove(pokedex_->getMoves().at("spikes"))
        .addMove(pokedex_->getMoves().at("toxic spikes"))
        .addMove(pokedex_->getMoves().at("rapid spin"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile() // normal pokemon
        .setBase(pokedex_->getPokemon().at("rattata"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile() // has levitate
        .setBase(pokedex_->getPokemon().at("azelf"))
        .setAbility(pokedex_->getAbilities().at("levitate"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile() // is flying type
        .setBase(pokedex_->getPokemon().at("pidgey"))
        .setLevel(100))
      .initialize();
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto stealth_rock = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  auto spikes = engine_->updateState(engine_->initialState(), Action::move(1), Action::wait());
  auto toxic_spikes = engine_->updateState(engine_->initialState(), Action::move(2), Action::wait());
  
  { // test rapid-spin removal:
    auto spikes_removed = engine_->updateState(spikes.at(0), Action::wait(), Action::move(3));
    auto removed_vs_spikes = engine_->updateState(spikes_removed.at(0), Action::wait(), Action::swap(1));
    EXPECT_EQ(removed_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 1.); // 100%
  }
  { // test normal harmed vs spikes:
    auto normal_vs_spikes = engine_->updateState(spikes.at(0), Action::wait(), Action::swap(1));
    EXPECT_NEAR(normal_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 0.875, 0.005); // 87.5%
  }
  { // test normal harmed vs toxic spikes:
    auto normal_vs_toxic = engine_->updateState(toxic_spikes.at(0), Action::wait(), Action::swap(1));
    EXPECT_NEAR(normal_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 0.875, 0.005); // 87.5%
    EXPECT_EQ(normal_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getStatusAilment(), AIL_NV_POISON); // 87.5%
  }
  { // test levitate unharmed vs spikes:
    auto lev_vs_spikes = engine_->updateState(spikes.at(0), Action::wait(), Action::swap(2));
    EXPECT_EQ(lev_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 1.); // 100%
  }
  { // test levitate unharmed vs toxic spikes:
    auto lev_vs_toxic = engine_->updateState(toxic_spikes.at(0), Action::wait(), Action::swap(2));
    EXPECT_EQ(lev_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 1.); // 100%
  }
  { // test levitate harmed vs stealth rock:
    auto lev_vs_sr = engine_->updateState(stealth_rock.at(0), Action::wait(), Action::swap(2));
    EXPECT_NEAR(lev_vs_sr.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 0.875, 0.005); // 87.5%
  }
  { // test flying unharmed vs spikes:
    auto flying_vs_spikes = engine_->updateState(spikes.at(0), Action::wait(), Action::swap(3));
    EXPECT_EQ(flying_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 1.); // 100%
  }
  { // test flying harmed vs stealth rock:
    auto flying_vs_sr = engine_->updateState(stealth_rock.at(0), Action::wait(), Action::swap(3));
    EXPECT_NEAR(flying_vs_sr.at(0).getEnv().getTeam(1).getPKV().getPercentHP(), 0.75, 0.005); // 75%
  }
}


TEST_F(EngineTest, HiddenPower) {
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


TEST_F(EngineTest, LifeOrb) {
  auto team_1 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("dusknoir"))
        .addMove(pokedex_->getMoves().at("pain split"))
        .addMove(pokedex_->getMoves().at("shadow punch"))
        .addMove(pokedex_->getMoves().at("will-o-wisp"))
        .addMove(pokedex_->getMoves().at("calm mind"))
        .setInitialItem(pokedex_->getItems().at("life orb"))
        .setLevel(100));
  auto team_2 = team_1;
  team_2.teammate(0).setNoInitialItem();
  auto environment = EnvironmentNonvolatile(team_1, team_2, true);
  engine_->setEnvironment(environment);

  auto sp_lifeorb = engine_->updateState(engine_->initialState(), Action::move(1), Action::wait());
  auto sp_noitem = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(1));
  auto will_o_wisp = engine_->updateState(engine_->initialState(), Action::move(2), Action::wait());
  auto split_pain = engine_->updateState(sp_lifeorb.at(0), Action::move(0), Action::wait());
  auto calm_mind = engine_->updateState(engine_->initialState(), Action::move(3), Action::wait());

  { // attacking move: life orb subtracts 10%, damage is increased by 30%
    EXPECT_EQ(sp_lifeorb.at(0).getEnv().getTeam(0).teammate(0).getHP(), 180); // 90%
    EXPECT_EQ(sp_lifeorb.at(0).getEnv().getTeam(1).teammate(0).getHP(), 62); // 31%
    EXPECT_LT(sp_lifeorb.at(0).getEnv().getTeam(1).teammate(0).getHP(),
              sp_noitem.at(0).getEnv().getTeam(0).teammate(0).getHP());
  }
  { // special move targeting other team: no effect
    EXPECT_EQ(split_pain.at(0).getEnv().getTeam(0).teammate(0).getHP(),
              split_pain.at(0).getEnv().getTeam(1).teammate(0).getHP());
  }
  { // status move targeting other team: no effect
    EXPECT_EQ(will_o_wisp.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 1.);
  }
  { // move targeting friendly team: no effect
    EXPECT_EQ(calm_mind.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 1.);
  }
}


TEST_F(EngineTest, ChoiceItems) {
  auto team_1 = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("scizor"))
        .addMove(pokedex_->getMoves().at("bullet punch")) // increased by choice band
        .addMove(pokedex_->getMoves().at("swift")) // not increased
        .setInitialItem(pokedex_->getItems().at("choice band"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("azelf"))
        .addMove(pokedex_->getMoves().at("swift")) // increased by choice scarf
        .addMove(pokedex_->getMoves().at("fire punch")) // not increased
        .setInitialItem(pokedex_->getItems().at("choice specs"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->getPokemon().at("flygon"))
        .addMove(pokedex_->getMoves().at("draco meteor"))
        .setInitialItem(pokedex_->getItems().at("choice scarf"))
        .setLevel(100));
  auto team_2 = team_1;
  team_2.teammate(0).setNoInitialItem();
  team_2.teammate(1).setNoInitialItem();
  team_2.teammate(2).setNoInitialItem();
  auto environment = EnvironmentNonvolatile(team_1, team_2, true);
  engine_->setEnvironment(environment);

  auto bulletpunch_cb = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto swift_cb = engine_->updateState(engine_->initialState(), Action::move(1), Action::move(1));

  auto azelf_pair = engine_->updateState(engine_->initialState(), Action::swap(1), Action::swap(1));
  auto swift_cs = engine_->updateState(azelf_pair.at(0), Action::move(0), Action::move(0));
  auto firepunch_cs = engine_->updateState(azelf_pair.at(0), Action::move(1), Action::move(1));

  auto flygon_pair = engine_->updateState(engine_->initialState(), Action::swap(2), Action::swap(2));
  auto dracometeor_cs = engine_->updateState(flygon_pair.at(0), Action::move(0), Action::move(0));
  auto dracometeor_none = engine_->updateState(flygon_pair.at(0), Action::wait(), Action::move(0));

  { // other moves are locked out after using a choice move:
    EXPECT_TRUE(engine_->isValidAction(bulletpunch_cb.at(0), Action::move(0), TEAM_A));
    EXPECT_FALSE(engine_->isValidAction(bulletpunch_cb.at(0), Action::move(1), TEAM_A));
  }
  { // when all PP have been used, only struggle is available:
    auto noPPState = bulletpunch_cb.at(1);
    noPPState.getEnv().getTeam(0).getPKV().getMV(0).setPP(0);
    EXPECT_FALSE(engine_->isValidAction(noPPState, Action::move(0), TEAM_A)); // locked due to PP
    EXPECT_FALSE(engine_->isValidAction(noPPState, Action::move(1), TEAM_A)); // locked due to Choice
    EXPECT_TRUE(engine_->isValidAction(noPPState, Action::struggle(), TEAM_A));
  }
  { // physical attack with choice band deals additional damage:
    EXPECT_GT(bulletpunch_cb.at(0).getEnv().getTeam(0).teammate(0).getHP(),
              bulletpunch_cb.at(0).getEnv().getTeam(1).teammate(0).getHP());
  }
  { // special attack with choice band does no additional damage:
    EXPECT_EQ(swift_cb.at(0).getEnv().getTeam(0).teammate(0).getHP(),
              swift_cb.at(0).getEnv().getTeam(1).teammate(0).getHP());
  }
  { // special attack with choice specs deals additional damage:
    EXPECT_GT(swift_cs.at(0).getEnv().getTeam(0).teammate(1).getHP(),
              swift_cs.at(0).getEnv().getTeam(1).teammate(1).getHP());
  }
  { // physical attack with choice specs does no additional damage:
    EXPECT_EQ(firepunch_cs.at(0).getEnv().getTeam(0).teammate(1).getHP(),
              firepunch_cs.at(0).getEnv().getTeam(1).teammate(1).getHP());
  }
  { // speed boost with 1-hit KO moves before enemy can deal damage with choice scarf:
    EXPECT_EQ(dracometeor_cs.at(0).getEnv().getTeam(0).teammate(2).getPercentHP(), 1.);
    EXPECT_GE(dracometeor_cs.at(0).getProbability().to_double(), 0.89); // enemy never moves
    EXPECT_EQ(dracometeor_cs.at(0).getEnv().getTeam(1).teammate(2).getHP(), 0);
    EXPECT_EQ(dracometeor_none.at(0).getEnv().getTeam(0).teammate(2).getHP(), 0);
  }
}


TEST_F(EngineTest, Pursuit) {
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


TEST_F(EngineTest, Outrage) {
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


TEST_F(EngineTest, UTurn) {
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

  auto setup_swap = engine_->updateState(engine_->initialState(), Action::wait(), Action::swap(1));
  auto setup_sr = engine_->updateState(setup_swap.at(0), Action::wait(), Action::move(0));
  auto uturn_to_ally = engine_->updateState(setup_sr.at(0), Action::moveAlly(0, 1), Action::wait());
  auto swap_to_scyzor = engine_->updateState(uturn_to_ally.at(0), Action::wait(), Action::swap(0));
  auto uturn_no_ally = engine_->updateState(swap_to_scyzor.at(0), Action::wait(), Action::moveAlly(0, 0));

  // TODO(@drendleman) test that using Action::move(0) is supported and picks the first viable pokemon
  //EXPECT_DEATH(engine_->updateState(swap_to_scyzor.at(0), Action::nothing(), Action::move(0)));

  { // swap condition with an ally:
    EXPECT_FALSE(engine_->isValidAction(setup_sr.at(0), Action::move(0), TEAM_A));
    EXPECT_TRUE(engine_->isValidAction(setup_sr.at(0), Action::moveAlly(0, 1), TEAM_A));
    EXPECT_FALSE(engine_->isValidAction(setup_sr.at(0), Action::moveAlly(0, 0), TEAM_A));
  }
  { // swap condition with no ally:
    EXPECT_FALSE(engine_->isValidAction(swap_to_scyzor.at(0), Action::move(0), TEAM_B));
    EXPECT_TRUE(engine_->isValidAction(swap_to_scyzor.at(0), Action::moveAlly(0, 0), TEAM_B));
    EXPECT_FALSE(engine_->isValidAction(swap_to_scyzor.at(0), Action::moveAlly(0, 1), TEAM_B));
  }
  { // u-turn with an ally:
    EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 31); // pp decremented
    EXPECT_NEAR(uturn_to_ally.at(0).getEnv().getTeam(0).teammate(0).getPercentHP(), 0.7875, 0.005); // item effect AND stealth-rock apply
    EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(0).getICPKV(), 1); // ally has swapped out
    EXPECT_EQ(uturn_to_ally.at(0).getEnv().getTeam(1).teammate(1).getPercentHP(), 0.); // enemy weakling deleted
  }
  { // u-turn with no ally:
    EXPECT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).teammate(0).getMV(0).getPP(), 31);
    EXPECT_FLOAT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 0.9); // item effect (life orb) applies
    EXPECT_EQ(uturn_no_ally.at(0).getEnv().getTeam(1).getICPKV(), 0); // ally NOT swapped out
  }
}


TEST_F(EngineTest, SuckerPunch) {
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

  auto suckerpunch_dmg = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(2));
  auto suckerpunch_status = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(1));
  auto suckerpunch_move = engine_->updateState(engine_->initialState(), Action::move(0), Action::swap(1));

  { // no damage to pokemon who used status:
    EXPECT_EQ(suckerpunch_status.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 1.);
    EXPECT_EQ(suckerpunch_status.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
  }
  { // no damage to the pokemon who swapped:
    EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 1.);
    EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(1).teammate(1).getPercentHP(), 1.);
    EXPECT_EQ(suckerpunch_move.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
  }
  {  // full damage to pokemon who attacked:
    EXPECT_LT(suckerpunch_dmg.at(0).getEnv().getTeam(1).teammate(0).getPercentHP(), 0.75);
    EXPECT_EQ(suckerpunch_dmg.at(0).getEnv().getTeam(0).teammate(0).getMV(0).getPP(), 7);
  }
}
