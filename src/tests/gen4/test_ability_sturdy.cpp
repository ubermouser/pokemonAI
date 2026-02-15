#include "engine_test.hpp"

class SturdyTest : public Gen4EngineTest {
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
          .setLevel(100));

    // Team B: Pokemon with and without Sturdy
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("skarmory")) // Target 0: Has Sturdy
          .setAbility(pokedex_->ability("sturdy"))
          .addMove(pokedex_->move("peck"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle")) // Target 1: No Sturdy
          .setAbility(pokedex_->ability("technician"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(50));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(SturdyTest, ImmuneToHornDrill) {
  auto state = engine_->initialState();
  // Smeargle uses Horn Drill (Move 0) against Skarmory (Sturdy)
  auto results = engine_->updateState(state, Action::move(0), Action::wait());
  EXPECT_TRUE(results.whereHit(0).empty());
}

TEST_F(SturdyTest, ImmuneToGuillotine) {
  auto state = engine_->initialState();
  // Smeargle uses Guillotine (Move 1) against Skarmory (Sturdy)
  auto results = engine_->updateState(state, Action::move(1), Action::wait());
  EXPECT_TRUE(results.whereHit(0).empty());
}

TEST_F(SturdyTest, ImmuneToFissure) {
  auto state = engine_->initialState();
  // Smeargle uses Fissure (Move 2) against Skarmory (Sturdy)
  // Note: Skarmory is Flying type, so it's already immune, but Sturdy should also apply.
  auto results = engine_->updateState(state, Action::move(2), Action::wait());
  EXPECT_TRUE(results.whereHit(0).empty());
}

TEST_F(SturdyTest, ImmuneToSheerCold) {
  auto state = engine_->initialState();
  // Smeargle uses Sheer Cold (Move 3) against Skarmory (Sturdy)
  auto results = engine_->updateState(state, Action::move(3), Action::wait());
  EXPECT_TRUE(results.whereHit(0).empty());
}

TEST_F(SturdyTest, NotImmuneWithoutSturdy) {
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};
  // Switch to Target 1 (Technician Smeargle)
  state.getOtherTeam(TEAM_A).swapPokemon(1);

  // Smeargle uses Horn Drill (Move 0)
  auto results = engine_->updateState(state, Action::move(0), Action::wait());
  EXPECT_FALSE(results.whereHit(0).empty());
}
