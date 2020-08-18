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
