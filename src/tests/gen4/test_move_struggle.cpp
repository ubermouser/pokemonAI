#include "engine_test.hpp"
#include "pokemonai/environment_possible.h"

class StruggleTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // Team A: Alakazam with one move (Psychic)
    auto team_a = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("alakazam"))
            .addMove(pokedex_->move("psychic"))
            .setLevel(100));

    // Team B: Gengar (Ghost type)
    auto team_b = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("gengar"))
            .addMove(pokedex_->move("shadow ball"))
            .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    // Use wait actions to get a mutable PossibleEnvironments from the engine
    state_no_pp_data = engine_->initialState().data();
    // Set PP of Psychic to 0 to force Struggle
    state_no_pp_data.teams[0].teammates[0].actions[0].PPcurrent = 0;
  }

  EnvironmentVolatileData state_no_pp_data;
};


TEST_F(StruggleTest, ForcedWhenNoPP) {
  auto state_no_pp = EnvironmentVolatile{engine_->initialState().nv(), state_no_pp_data};

  // Psychic should be invalid
  EXPECT_FALSE(engine_->isValidAction(state_no_pp, Action::move(0), TEAM_A));
  
  // Struggle should be valid (and actually the only valid move)
  EXPECT_TRUE(engine_->isValidAction(state_no_pp, Action::struggle(), TEAM_A));
}


TEST_F(StruggleTest, HitsGhostTypes) {
  auto state_no_pp = EnvironmentVolatile{engine_->initialState().nv(), state_no_pp_data};

  // Turn 1: Alakazam uses Struggle, Gengar uses wait
  auto results = engine_->updateState(state_no_pp, Action::struggle(), Action::wait());
  auto next_state = results.where1();

  // Gengar should have taken damage
  EXPECT_LT(next_state.teammate(1, 0).getHP(), next_state.teammate(1, 0).nv().getMaxHP());
}


TEST_F(StruggleTest, RecoilDamage) {
  auto state_no_pp = EnvironmentVolatile{engine_->initialState().nv(), state_no_pp_data};

  auto initial_hp = state_no_pp.teammate(0, 0).getHP();
  auto max_hp = state_no_pp.teammate(0, 0).nv().getMaxHP();

  auto results = engine_->updateState(state_no_pp, Action::struggle(), Action::wait());
  auto next_state = results.where1();

  // Alakazam should have taken 25% recoil
  int expected_recoil = static_cast<int>(max_hp * 0.25);
  EXPECT_EQ(next_state.teammate(0, 0).getHP(), initial_hp - expected_recoil);
}
