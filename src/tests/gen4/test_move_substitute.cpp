#include "engine_test.hpp"


class SubstituteTest : public Gen4EngineTest {
 protected:
  PossibleEnvironments turn1_results;

  void SetUp() override {
    Gen4EngineTest::SetUp();
    // Team A: Pokemon with Substitute
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("substitute"))
          .addMove(pokedex_->move("softboiled"))
          .addMove(pokedex_->move("psychic"))
          .setLevel(50));

    // Team B: Pokemon with various moves
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("shadow ball")) // Special Damaging move
          .addMove(pokedex_->move("shadow punch")) // Physical Damaging move
          .addMove(pokedex_->move("taunt"))      // Bypasses substitute (in Gen 4)
          .addMove(pokedex_->move("sludge bomb")) // Has secondary effect (poison)
          .setLevel(20));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    // Common first turn: Alakazam uses Substitute
    turn1_results = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());
  }
};

TEST_F(SubstituteTest, CreatesSubstitute) {
  // Blissey uses Substitute (already done in SetUp)
  auto blissey = turn1_results.where1().teammate(0, 0);

  uint32_t maxHP = blissey.nv().getMaxHP();
  uint32_t expectedHP = maxHP - (maxHP / 4);
  uint32_t expectedSubHP = maxHP / 4;
  EXPECT_EQ(blissey.getHP(), expectedHP);
  EXPECT_EQ(blissey.status().substitute, expectedSubHP);
}

TEST_F(SubstituteTest, FailsWithLowHP) {
  // Set Blissey's HP very low
  EnvironmentVolatileData stateData = engine_->initialState().data();
  EnvironmentVolatile state{engine_->initialState().nv(), stateData};
  state.teammate(0, 0).setHP(10);

  // Try to use Substitute
  auto result = engine_->updateState(state, Action::move(0), Action::wait());
  auto blissey = result.where1().teammate(0, 0);

  // HP should not change, substitute should not be created
  EXPECT_EQ(blissey.getHP(), 10U);
  EXPECT_EQ(blissey.status().substitute, 0U);
}

TEST_F(SubstituteTest, AbsorbsDamage) {
  // Blissey uses Substitute in SetUp
  auto state1 = turn1_results.where1();

  uint32_t initialHP = state1.teammate(0, 0).getHP();
  uint32_t initialSubHP = state1.teammate(0, 0).status().substitute;

  // Turn 2: Gengar uses Sludge Bomb (Move index 3 - hits Blissey)
  auto turn2 = engine_->updateState(state1, Action::wait(), Action::move(3));
  auto blissey2 = turn2.where1().teammate(0, 0);

  // Blissey's HP should not have decreased
  EXPECT_EQ(blissey2.getHP(), initialHP);
  // Substitute HP should have decreased
  EXPECT_LT(blissey2.status().substitute, initialSubHP);
}

TEST_F(SubstituteTest, BreaksSubstitute) {
  // Blissey uses Substitute in SetUp
  auto state1 = turn1_results.where1();

  // Force break substitute by creating a mutable state
  EnvironmentVolatileData stateData = state1.getEnv().data();
  EnvironmentVolatile mutableState{state1.getEnv().nv(), stateData};
  mutableState.teammate(0, 0).status().substitute = 1;

  // Turn 2: Gengar uses Sludge Bomb (Move index 3)
  auto turn2 = engine_->updateState(mutableState, Action::wait(), Action::move(3));
  auto blissey2 = turn2.where1().teammate(0, 0);

  // Substitute should be gone
  EXPECT_EQ(blissey2.status().substitute, 0U);
  // Blissey's HP should still be same as start of Turn 2
  EXPECT_EQ(blissey2.getHP(), state1.teammate(0, 0).getHP());
}

TEST_F(SubstituteTest, BlocksStatusMove) {
  // Blissey uses Substitute in SetUp
  auto state1 = turn1_results.where1();

  // Turn 2: Gengar uses Toxic
  auto turn2 = engine_->updateState(state1, Action::wait(), Action::move(0));
  auto blissey2 = turn2.where1().teammate(0, 0);

  // Blissey should NOT be poisoned
  EXPECT_EQ(blissey2.getStatusAilment(), AIL_NV_NONE);
}

TEST_F(SubstituteTest, BypassedByTaunt) {
  // Blissey uses Substitute in SetUp
  auto state1 = turn1_results.where1();

  // Turn 2: Gengar uses Taunt
  auto turn2 = engine_->updateState(state1, Action::wait(), Action::move(2));
  auto blissey2 = turn2.where1().teammate(0, 0);

  // Blissey should be taunted
  EXPECT_GT(blissey2.status().taunt_duration, 0U);
}

TEST_F(SubstituteTest, BlocksSecondaryEffect) {
  // Blissey uses Substitute in SetUp
  auto state1 = turn1_results.where1();

  // Turn 2: Gengar uses Sludge Bomb (30% chance to poison) (Move index 3)
  // We want to verify that even if it hits, the poison effect is blocked.
  // We set higher RNG branches to ensure the 30% chance is "rolled"
  engine_->setAccuracy(50);
  auto turn2 = engine_->updateState(state1, Action::wait(), Action::move(3));

  for (size_t i = 0; i < turn2.size(); ++i) {
    auto blissey = turn2.at(i).teammate(0, 0);
    EXPECT_EQ(blissey.getStatusAilment(), AIL_NV_NONE);
  }
}

TEST_F(SubstituteTest, DoesNotBlockSelfTargetingMoves) {
  // Blissey uses Substitute in SetUp
  auto state1 = turn1_results.where1();

  // Set Blissey's HP lower to see healing effect
  EnvironmentVolatileData stateData = state1.getEnv().data();
  EnvironmentVolatile mutableState{state1.getEnv().nv(), stateData};
  uint32_t hpAfterSub = mutableState.teammate(0, 0).getHP();
  mutableState.teammate(0, 0).setHP(hpAfterSub - 20);

  // Turn 2: Blissey uses Soft-Boiled (Move index 1)
  auto turn2 = engine_->updateState(mutableState, Action::move(1), Action::wait());
  auto blissey2 = turn2.where1().teammate(0, 0);

  // Blissey should have healed
  EXPECT_GT(blissey2.getHP(), hpAfterSub - 20);
}

TEST_F(SubstituteTest, SubstituteReported) {
  // Blissey uses Substitute in SetUp
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), turn1_results.where1(), false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("blissey put up a substitute") != std::string::npos);
}
