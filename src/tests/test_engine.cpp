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
      engine_->initialState(), AT_MOVE_0, AT_MOVE_NOTHING);

  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.at(0).hasHit(0), true);
  EXPECT_EQ(result.at(1).hasHit(0), false);
  EXPECT_EQ(result.at(2).hasCrit(0), true);
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

  auto pikachu_poisoned = engine_->updateState(engine_->initialState(), AT_MOVE_NOTHING, AT_MOVE_0);
  auto blissey_poisoned = engine_->updateState(pikachu_poisoned.at(0), AT_SWITCH_1, AT_MOVE_0);
  auto all_cured = engine_->updateState(blissey_poisoned.at(0), AT_MOVE_0, AT_MOVE_NOTHING);

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

  auto pidgey_tackled = engine_->updateState(engine_->initialState(), AT_MOVE_NOTHING, AT_MOVE_0);
  auto pidgey_roosted = engine_->updateState(pidgey_tackled.at(1), AT_MOVE_1, AT_MOVE_NOTHING);

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

  auto stealth_rock = engine_->updateState(engine_->initialState(), AT_MOVE_0, AT_MOVE_NOTHING);
  auto spikes = engine_->updateState(engine_->initialState(), AT_MOVE_1, AT_MOVE_NOTHING);
  auto toxic_spikes = engine_->updateState(engine_->initialState(), AT_MOVE_2, AT_MOVE_NOTHING);
  
  { // test rapid-spin removal:
    auto spikes_removed = engine_->updateState(spikes.at(0), AT_MOVE_NOTHING, AT_MOVE_3);
    auto removed_vs_spikes = engine_->updateState(spikes_removed.at(0), AT_MOVE_NOTHING, AT_SWITCH_1);
    EXPECT_EQ(removed_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getHP(), 170); // 100%
  }
  { // test normal harmed vs spikes:
    auto normal_vs_spikes = engine_->updateState(spikes.at(0), AT_MOVE_NOTHING, AT_SWITCH_1);
    EXPECT_EQ(normal_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getHP(), 149); // 87.5%
  }
  { // test normal harmed vs toxic spikes:
    auto normal_vs_toxic = engine_->updateState(toxic_spikes.at(0), AT_MOVE_NOTHING, AT_SWITCH_1);
    EXPECT_EQ(normal_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getHP(), 149); // 87.5%
    EXPECT_EQ(normal_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getStatusAilment(), AIL_NV_POISON); // 87.5%
  }
  { // test levitate unharmed vs spikes:
    auto lev_vs_spikes = engine_->updateState(spikes.at(0), AT_MOVE_NOTHING, AT_SWITCH_2);
    EXPECT_EQ(lev_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getHP(), 260); // 100%
  }
  { // test levitate unharmed vs toxic spikes:
    auto lev_vs_toxic = engine_->updateState(toxic_spikes.at(0), AT_MOVE_NOTHING, AT_SWITCH_2);
    EXPECT_EQ(lev_vs_toxic.at(0).getEnv().getTeam(1).getPKV().getHP(), 260); // 100%
  }
  { // test levitate harmed vs stealth rock:
    auto lev_vs_sr = engine_->updateState(stealth_rock.at(0), AT_MOVE_NOTHING, AT_SWITCH_2);
    EXPECT_EQ(lev_vs_sr.at(0).getEnv().getTeam(1).getPKV().getHP(), 260); // 87.5%
  }
  { // test flying unharmed vs spikes:
    auto flying_vs_spikes = engine_->updateState(spikes.at(0), AT_MOVE_NOTHING, AT_SWITCH_3);
    EXPECT_EQ(flying_vs_spikes.at(0).getEnv().getTeam(1).getPKV().getHP(), 190); // 100%
  }
  { // test flying harmed vs stealth rock:
    auto flying_vs_sr = engine_->updateState(stealth_rock.at(0), AT_MOVE_NOTHING, AT_SWITCH_3);
    EXPECT_EQ(flying_vs_sr.at(0).getEnv().getTeam(1).getPKV().getHP(), 143); // 75%
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

  auto hidden_power = engine_->updateState(engine_->initialState(), AT_MOVE_0, AT_MOVE_NOTHING);

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

  auto sp_lifeorb = engine_->updateState(engine_->initialState(), AT_MOVE_1, AT_MOVE_NOTHING);
  auto sp_noitem = engine_->updateState(engine_->initialState(), AT_MOVE_NOTHING, AT_MOVE_1);
  auto will_o_wisp = engine_->updateState(engine_->initialState(), AT_MOVE_2, AT_MOVE_NOTHING);
  auto split_pain = engine_->updateState(sp_lifeorb.at(0), AT_MOVE_0, AT_MOVE_NOTHING);
  auto calm_mind = engine_->updateState(engine_->initialState(), AT_MOVE_3, AT_MOVE_NOTHING);

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

  auto bulletpunch_cb = engine_->updateState(engine_->initialState(), AT_MOVE_0, AT_MOVE_0);
  auto swift_cb = engine_->updateState(engine_->initialState(), AT_MOVE_1, AT_MOVE_1);

  auto azelf_pair = engine_->updateState(engine_->initialState(), AT_SWITCH_1, AT_SWITCH_1);
  auto swift_cs = engine_->updateState(azelf_pair.at(0), AT_MOVE_0, AT_MOVE_0);
  auto firepunch_cs = engine_->updateState(azelf_pair.at(0), AT_MOVE_1, AT_MOVE_1);

  auto flygon_pair = engine_->updateState(engine_->initialState(), AT_SWITCH_2, AT_SWITCH_2);
  auto dracometeor_cs = engine_->updateState(flygon_pair.at(0), AT_MOVE_0, AT_MOVE_0);
  auto dracometeor_none = engine_->updateState(flygon_pair.at(0), AT_MOVE_NOTHING, AT_MOVE_0);

  { // other moves are locked out after using a choice move:
    EXPECT_EQ(engine_->isValidAction(bulletpunch_cb.at(0), AT_MOVE_0, TEAM_A), true);
    EXPECT_EQ(engine_->isValidAction(bulletpunch_cb.at(0), AT_MOVE_1, TEAM_A), false);
  }
  { // when all PP have been used, only struggle is available:
    auto noPPState = bulletpunch_cb.at(1);
    noPPState.getEnv().getTeam(0).getPKV().getMV(0).setPP(0);
    EXPECT_EQ(engine_->isValidAction(noPPState, AT_MOVE_0, TEAM_A), false); // locked due to PP
    EXPECT_EQ(engine_->isValidAction(noPPState, AT_MOVE_1, TEAM_A), false); // locked due to Choice
    EXPECT_EQ(engine_->isValidAction(noPPState, AT_MOVE_STRUGGLE, TEAM_A), true);
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
