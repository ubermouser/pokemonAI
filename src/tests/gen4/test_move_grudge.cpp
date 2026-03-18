#include "engine_test.hpp"
#include "pokemonai/pkai.h"


class GrudgeTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    pokedex_->setAllowInvalidPokemon(true);

    // Misdreavus (Ghost) is fast (85) and learns Grudge.
    // clang-format off
    auto teamA = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("misdreavus"))
          .addMove(pokedex_->move("grudge"))
          .addMove(pokedex_->move("shadow sneak"))
          .setLevel(100));

    // Gardevoir is slower (80) and learns Psychic, Confuse Ray, and Sheer Cold.
    auto teamB = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gardevoir"))
          .addMove(pokedex_->move("psychic"))
          .addMove(pokedex_->move("calm mind"))
          .addMove(pokedex_->move("sheer cold"))
          .setLevel(100));
    // clang-format on

    environment_nv = EnvironmentNonvolatile(teamA, teamB, true);
    engine_->setEnvironment(environment_nv);
  }
};


TEST_F(GrudgeTest, DepletesPPOnKnockout) {
  // Misdreavus (fast) uses Grudge. Gardevoir (slow) uses Psychic.
  auto turn = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));

  auto state = turn.where1Crit(1);

  EXPECT_FALSE(state.teammate(0, 0).isAlive());
  EXPECT_EQ(state.teammate(1, 0).getMV(0).getPP(), 0);
}


TEST_F(GrudgeTest, WearsOffAfterNextMove) {
  // Turn 1: Misdreavus uses Grudge. Gardevoir uses Calm Mind (non-damaging).
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(1));

  auto state1 = turn1.where1Hit(0);
  EXPECT_TRUE(state1.teammate(0, 0).status().cTeammate.grudge);

  // Turn 2: Misdreavus uses Shadow Sneak (priority). Gardevoir uses Psychic.
  // Grudge should clear at the start of Turn 2 for Misdreavus.
  auto turn2 = engine_->updateState(state1, Action::move(1), Action::move(0));

  auto state2 = turn2.where1Hit(1);

  EXPECT_FALSE(state2.teammate(0, 0).isAlive());
  // Gardevoir's Psychic should still have PP because Grudge wore off.
  EXPECT_GT(state2.teammate(1, 0).getMV(0).getPP(), 0);
}


TEST_F(GrudgeTest, DISABLED_TriggersOnOHKOMoves) {
  // OHKO moves like Sheer Cold also cause Grudge to trigger if they KO.
  // Misdreavus uses Grudge. Gardevoir uses Sheer Cold.
  auto turn = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(2));

  auto state = turn.where1Hit(1);

  // Gardevoir's Sheer Cold (move 2) should have 0 PP.
  EXPECT_EQ(state.teammate(1, 0).getMV(2).getPP(), 0);
}
