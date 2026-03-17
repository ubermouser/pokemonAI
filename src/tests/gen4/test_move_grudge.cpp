#include "engine_test.hpp"

class GrudgeTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Misdreavus (Ghost) is fast (85) and learns Grudge.
    // clang-format off
    auto teamA = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("misdreavus"))
          .addMove(pokedex_->move("grudge"))
          .addMove(pokedex_->move("shadow sneak"))
          .setLevel(50));

    // Lapras is slower (60) and learns Sheer Cold, Confuse Ray, and Ice Beam.
    auto teamB = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("lapras"))
          .addMove(pokedex_->move("ice beam"))
          .addMove(pokedex_->move("confuse ray"))
          .addMove(pokedex_->move("sheer cold"))
          .setLevel(100));
    // clang-format on

    environment_nv = EnvironmentNonvolatile(teamA, teamB, true);
    engine_->setEnvironment(environment_nv);
  }
};

TEST_F(GrudgeTest, DISABLED_DepletesPPOnKnockout) {
  // Misdreavus (fast) uses Grudge. Lapras (slow) uses Ice Beam.
  auto turn = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));

  auto state = turn.where1HitNoCrit(1);
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), state, false);
  SCOPED_TRACE(output);

  EXPECT_FALSE(state.teammate(0, 0).isAlive());
  EXPECT_EQ(state.teammate(1, 0).getMV(0).getPP(), 0);
}

TEST_F(GrudgeTest, DISABLED_WearsOffAfterNextMove) {
  // Turn 1: Misdreavus uses Grudge. Lapras uses Confuse Ray (non-damaging).
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(1));
  auto state1 = turn1.where1Hit(0);
  EXPECT_TRUE(state1.teammate(0, 0).status().cTeammate.grudge);

  // Turn 2: Misdreavus uses Shadow Sneak (priority). Duskull uses Shadow Ball.
  // Grudge should clear at the start of Turn 2 for Misdreavus.
  auto turn2 = engine_->updateState(state1, Action::move(1), Action::move(0));
  auto state2 = turn2.where1Hit(1);
  
  EXPECT_FALSE(state2.teammate(0, 0).isAlive());
  // Duskull's Shadow Ball should still have PP because Grudge wore off.
  EXPECT_GT(state2.teammate(1, 0).getMV(0).getPP(), 0);
}

TEST_F(GrudgeTest, DISABLED_TriggersOnOHKOMoves) {
  // OHKO moves like Sheer Cold also cause Grudge to trigger if they KO.
  // Misdreavus uses Grudge. Duskull uses Sheer Cold.
  auto turn = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(2));

  // Branch where Sheer Cold hits (index 0 usually).
  // Note: OHKO moves have low accuracy (30% base).
  // We might need to check multiple branches or ensure it hits.
  auto state = turn.where1Hit(1);

  // Duskull's Sheer Cold (move 2) should have 0 PP.
  EXPECT_EQ(state.teammate(1, 0).getMV(2).getPP(), 0);
}
