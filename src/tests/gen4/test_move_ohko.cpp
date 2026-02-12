#include "engine_test.hpp"

class OHKOTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: Smeargle with all 4 OHKO moves
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("horn drill"))
          .addMove(pokedex_->move("guillotine"))
          .addMove(pokedex_->move("fissure"))
          .addMove(pokedex_->move("sheer cold"))
          .setLevel(50));

    // Team B: Unique species with verified implemented moves
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle")) // Target 0: Normal level 50
          .addMove(pokedex_->move("tackle"))
          .addMove(pokedex_->move("growl"))
          .addMove(pokedex_->move("quick attack"))
          .addMove(pokedex_->move("leer"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bidoof")) // Target 1: Higher level 51
          .addMove(pokedex_->move("tackle"))
          .addMove(pokedex_->move("growl"))
          .addMove(pokedex_->move("quick attack"))
          .addMove(pokedex_->move("hyper fang"))
          .setLevel(51))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))   // Target 2: Ghost
          .addMove(pokedex_->move("astonish"))
          .addMove(pokedex_->move("night shade"))
          .addMove(pokedex_->move("curse"))
          .addMove(pokedex_->move("shadow punch"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("staraptor"))// Target 3: Flying
          .addMove(pokedex_->move("aerial ace"))
          .addMove(pokedex_->move("agility"))
          .addMove(pokedex_->move("air cutter"))
          .addMove(pokedex_->move("quick attack"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bronzong")) // Target 4: Levitate
          .setAbility(pokedex_->ability("levitate"))
          .addMove(pokedex_->move("confusion"))
          .addMove(pokedex_->move("iron defense"))
          .addMove(pokedex_->move("block"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("glalie"))    // Target 5: Ice
          .addMove(pokedex_->move("bite"))
          .addMove(pokedex_->move("headbutt"))
          .addMove(pokedex_->move("ice shard"))
          .addMove(pokedex_->move("leer"))
          .setLevel(50));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(OHKOTest, FailsAgainstHigherLevel) {
  // Switch Target 1 in
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};
  state.getOtherTeam(TEAM_A).swapPokemon(1);

  // Smeargle uses Horn Drill
  auto results = engine_->updateState(state, Action::move(0), Action::wait());

  // Should always miss
  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_FALSE(results.at(i).hasHit(0));
  }
}

TEST_F(OHKOTest, AccuracyFormula) {
  // Target 0 (Level 50)
  auto state = engine_->initialState();

  // Accuracy = (50 - 50) + 30 = 30%
  auto results = engine_->updateState(state, Action::move(0), Action::wait());

  FixType hitProb(0);
  for (size_t i = 0; i < results.size(); ++i) {
    if (results.at(i).hasHit(0)) hitProb += results.at(i).getProbability();
  }
  EXPECT_NEAR((float)hitProb, 0.30f, 0.001f);
}

TEST_F(OHKOTest, AccuracyWithLevelDifference) {
  // Team A: Smeargle Level 60
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("smeargle"))
        .addMove(pokedex_->move("horn drill"))
        .addMove(pokedex_->move("guillotine"))
        .addMove(pokedex_->move("fissure"))
        .addMove(pokedex_->move("sheer cold"))
        .setLevel(60));

  // Team B: Smeargle Level 50
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("smeargle"))
        .addMove(pokedex_->move("tackle"))
        .addMove(pokedex_->move("growl"))
        .addMove(pokedex_->move("quick attack"))
        .addMove(pokedex_->move("leer"))
        .setLevel(50));

  EnvironmentNonvolatile env_nv(team_a, team_b, true);
  engine_->setEnvironment(env_nv);

  // Accuracy = (60 - 50) + 30 = 40%
  auto results = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  FixType hitProb(0);
  for (size_t i = 0; i < results.size(); ++i) {
    if (results.at(i).hasHit(0)) hitProb += results.at(i).getProbability();
  }
  EXPECT_NEAR((float)hitProb, 0.40f, 0.001f);
}

TEST_F(OHKOTest, GhostImmunity) {
  // Switch Gengar in
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};
  state.getOtherTeam(TEAM_A).swapPokemon(2);

  // Horn Drill (Move 0) and Guillotine (Move 1) should fail
  auto res0 = engine_->updateState(state, Action::move(0), Action::wait());
  for (size_t i = 0; i < res0.size(); ++i) EXPECT_FALSE(res0.at(i).hasHit(0));

  auto res1 = engine_->updateState(state, Action::move(1), Action::wait());
  for (size_t i = 0; i < res1.size(); ++i) EXPECT_FALSE(res1.at(i).hasHit(0));
}

TEST_F(OHKOTest, IdentifyBypassesGhostImmunity) {
  // Switch Gengar in
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};
  state.getOtherTeam(TEAM_A).swapPokemon(2);

  // Set identify on Gengar
  state.getOtherTeam(TEAM_A).getPKV().status().cTeammate.identify = 1;

  // Horn Drill should now have 30% accuracy
  auto results = engine_->updateState(state, Action::move(0), Action::wait());

  FixType hitProb(0);
  for (size_t i = 0; i < results.size(); ++i) {
    if (results.at(i).hasHit(0)) hitProb += results.at(i).getProbability();
  }
  EXPECT_NEAR((float)hitProb, 0.30f, 0.001f);
}

TEST_F(OHKOTest, FlyingImmunity) {
  // Switch Staraptor in
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};
  state.getOtherTeam(TEAM_A).swapPokemon(3);

  // Fissure (Move 2) should fail
  auto results = engine_->updateState(state, Action::move(2), Action::wait());
  for (size_t i = 0; i < results.size(); ++i) EXPECT_FALSE(results.at(i).hasHit(0));
}

TEST_F(OHKOTest, LevitateImmunity) {
  // Switch Target 4 (Bronzong) in
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};
  state.getOtherTeam(TEAM_A).swapPokemon(4);

  // Fissure (Move 2) should fail
  auto results = engine_->updateState(state, Action::move(2), Action::wait());
  for (size_t i = 0; i < results.size(); ++i) EXPECT_FALSE(results.at(i).hasHit(0));
}

TEST_F(OHKOTest, SheerColdHitsIceType) {
  // Switch Glalie in
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};
  state.getOtherTeam(TEAM_A).swapPokemon(5);

  // Sheer Cold (Move 3) should have 30% accuracy in Gen 4
  auto results = engine_->updateState(state, Action::move(3), Action::wait());

  FixType hitProb(0);
  for (size_t i = 0; i < results.size(); ++i) {
    if (results.at(i).hasHit(0)) hitProb += results.at(i).getProbability();
  }
  EXPECT_NEAR((float)hitProb, 0.30f, 0.001f);
}

TEST_F(OHKOTest, OHKOEffect) {
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};

  // Accuracy formula ensures 30% hit chance
  auto results = engine_->updateState(state, Action::move(0), Action::wait());

  bool foundDead = false;
  for (size_t i = 0; i < results.size(); ++i) {
    if (results.at(i).hasHit(0)) {
        auto target = results.at(i).teammate(1, 0);
        EXPECT_EQ(target.getHP(), 0U);
        foundDead = true;
    }
  }
  EXPECT_TRUE(foundDead);
}
