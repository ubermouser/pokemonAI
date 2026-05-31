#include "engine_test.hpp"


class SubstituteTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // clang-format off
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
          .addMove(pokedex_->move("toxic"))        // Status move (index 0)
          .addMove(pokedex_->move("knock off"))     // Low damage move (index 1)
          .addMove(pokedex_->move("taunt"))        // Bypasses substitute (in Gen 4) (index 2)
          .addMove(pokedex_->move("sludge bomb"))  // High damage move (index 3)
          .setLevel(70))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("seismic toss"))
          .setLevel(70));
    // clang-format on 

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupSubstitute() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupSubstituteWithLowHP() {
    auto turn1 = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(3)); // Sludge Bomb
    auto turn2 = engine_->updateState(turn1.where1().getEnv(), Action::wait(), Action::move(3));
    auto turn3 = engine_->updateState(turn2.where1().getEnv(), Action::wait(), Action::move(3));
    return engine_->updateState(turn3.where1().getEnv(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupGengarAttacks(const ConstEnvironmentVolatile& state, size_t gengarMoveIndex) {
    return engine_->updateState(state, Action::wait(), Action::move(gengarMoveIndex));
  }

  PossibleEnvironments setupSubstituteBroken() {
    auto turn1 = setupSubstitute();
    auto state1 = turn1.where1();
    // Gengar uses Sludge Bomb (Move 3) which deals > 82 damage and breaks the substitute naturally
    return engine_->updateState(state1.getEnv(), Action::wait(), Action::move(3));
  }

  PossibleEnvironments setupBlisseyHealingTurn() {
    auto turn1 = setupSubstitute();
    auto state1 = turn1.where1();
    return engine_->updateState(state1.getEnv(), Action::move(1), Action::wait());
  }

  PossibleEnvironments setupSubstituteWithSmeargle() {
    return engine_->updateState(engine_->initialState(), Action::move(0), Action::swap(1));
  }
};


TEST_F(SubstituteTest, CreatesSubstitute) {
  auto turn1_results = setupSubstitute();
  auto blissey = turn1_results.where1().teammate(TEAM_A, 0);

  uint32_t maxHP = blissey.nv().getMaxHP();
  uint32_t expectedHP = maxHP - (maxHP / 4);
  uint32_t expectedSubHP = maxHP / 4;
  EXPECT_EQ(blissey.getHP(), expectedHP);
  EXPECT_EQ(blissey.status().substitute, expectedSubHP);
}


TEST_F(SubstituteTest, FailsWithLowHP) {
  auto result = setupSubstituteWithLowHP();
  auto blissey = result.where1().teammate(TEAM_A, 0);

  // HP should not change, substitute should not be created.
  // Blissey starts at 330 HP and takes 3 Sludge Bombs, leaving her with low HP.
  EXPECT_GE(blissey.getHP(), 1U);
  EXPECT_LT(blissey.getHP(), 82U);
  EXPECT_EQ(blissey.status().substitute, 0U);
}


TEST_F(SubstituteTest, AbsorbsDamage) {
  auto turn1 = setupSubstitute();
  auto state1 = turn1.where1();

  uint32_t initialHP = state1.teammate(TEAM_A, 0).getHP();
  uint32_t initialSubHP = state1.teammate(TEAM_A, 0).status().substitute;

  // Turn 2: Gengar uses Knock Off (Move index 1 - deals [39, 47] damage, less than 82 substitute HP)
  auto turn2 = setupGengarAttacks(state1.getEnv(), 1);
  auto blissey2 = turn2.where1().teammate(TEAM_A, 0);

  // Blissey's HP should not have decreased
  EXPECT_EQ(blissey2.getHP(), initialHP);
  // Substitute HP should have decreased
  EXPECT_LT(blissey2.status().substitute, initialSubHP);
  EXPECT_GT(blissey2.status().substitute, 0U);
}


TEST_F(SubstituteTest, BreaksSubstitute) {
  auto turn1 = setupSubstitute();
  auto state1 = turn1.where1();
  auto turn2 = setupSubstituteBroken();
  auto blissey2 = turn2.where1().teammate(TEAM_A, 0);

  // Substitute should be gone
  EXPECT_EQ(blissey2.status().substitute, 0U);
  // Blissey's HP should still be same as start of Turn 2
  EXPECT_EQ(blissey2.getHP(), state1.teammate(TEAM_A, 0).getHP());
}


TEST_F(SubstituteTest, BlocksStatusMove) {
  auto turn1 = setupSubstitute();
  auto state1 = turn1.where1();

  // Turn 2: Gengar uses Toxic
  auto turn2 = setupGengarAttacks(state1.getEnv(), 0);
  auto blissey2 = turn2.where1().teammate(TEAM_A, 0);

  // Blissey should NOT be poisoned
  EXPECT_EQ(blissey2.getStatusAilment(), AIL_NV_NONE);
}


TEST_F(SubstituteTest, BypassedByTaunt) {
  auto turn1 = setupSubstitute();
  auto state1 = turn1.where1();

  // Turn 2: Gengar uses Taunt
  auto turn2 = setupGengarAttacks(state1.getEnv(), 2);
  auto blissey2 = turn2.where1().teammate(TEAM_A, 0);

  // Blissey should be taunted
  EXPECT_GT(blissey2.status().taunt_duration, 0U);
}


TEST_F(SubstituteTest, BlocksSecondaryEffect) {
  auto turn1 = setupSubstitute();
  auto state1 = turn1.where1();

  // Turn 2: Gengar uses Sludge Bomb (30% chance to poison) (Move index 3)
  // We want to verify that even if the secondary effect is rolled, the poison effect is blocked.
  auto turn2 = setupGengarAttacks(state1.getEnv(), 3);

  auto secondary_state = turn2.where1Status(TEAM_B);
  EXPECT_EQ(secondary_state.teammate(TEAM_A, 0).getStatusAilment(), AIL_NV_NONE);
}


TEST_F(SubstituteTest, DoesNotBlockSelfTargetingMoves) {
  auto turn1 = setupSubstitute();
  auto state1 = turn1.where1();
  uint32_t hpAfterSub = state1.teammate(TEAM_A, 0).getHP();

  // Turn 2: Blissey uses Soft-Boiled (Move index 1)
  auto turn2 = setupBlisseyHealingTurn();
  auto blissey2 = turn2.where1().teammate(TEAM_A, 0);

  // Blissey should have healed
  EXPECT_GT(blissey2.getHP(), hpAfterSub);
}


TEST_F(SubstituteTest, SubstituteReported) {
  auto turn1_results = setupSubstitute();
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), turn1_results.where1(), false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("blissey put up a substitute") != std::string::npos);
}


TEST_F(SubstituteTest, SubstituteFadedReported) {
  auto turn1 = setupSubstitute();
  auto state1 = turn1.where1();
  auto turn2 = setupSubstituteBroken();
  auto state2 = turn2.where1();

  auto output = StateTransitionPrinter::printString(
      state1.getEnv(), state2, false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("blissey's substitute faded") != std::string::npos);
}



TEST_F(SubstituteTest, BlocksLeveledDamageMoves) {
  auto turn2 = setupSubstituteWithSmeargle();
  auto state2 = turn2.where1();

  uint32_t initialHP = state2.teammate(TEAM_A, 0).getHP();
  uint32_t initialSubHP = state2.teammate(TEAM_A, 0).status().substitute;

  // Smeargle uses Seismic Toss
  auto turn3 = engine_->updateState(
      state2.getEnv(), Action::wait(), Action::move(0));
  auto blissey3 = turn3.where1().teammate(TEAM_A, 0);

  // Blissey's HP should not have decreased
  EXPECT_EQ(blissey3.getHP(), initialHP);
  // Substitute HP should have decreased by Smeargle's level (70)
  EXPECT_EQ(blissey3.status().substitute, initialSubHP - 70U);
}
