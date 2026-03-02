#include <gtest/gtest.h>
#include "gen4/engine_test.hpp"
#include "pokemonai/pkCU.h"

class AccuracyReproTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // Ensure the global pkdex pointer is set, as some code still relies on it.
    pkdex = pokedex_.get();
  }

  void TearDown() override {
    pkdex = nullptr;
    Gen4EngineTest::TearDown();
  }
};

/**
 * Demonstrates the rounding error where 0.75 * (4/3) results in a value
 * slightly less than 1.0 due to fixed-point truncation during conversion.
 *
 * Specifically:
 * 4/3 as double is 1.3333333333333333
 * Converting to FixType (2^27 precision) results in floor(1.333... * 2^27) = 178956970
 * 0.75 in FixType is 100663296
 * Multiplication: (178956970 * 100663296) >> 27 = 134217727
 * 1.0 in FixType is 134217728
 * Result is 1 unit less than 1.0.
 */
TEST_F(AccuracyReproTest, RoundingErrorDiscovery) {
  // Ensure aFV_base is initialized
  PokemonNonVolatile dummy;
  dummy.setBase(pokedex_->pokemon("dragonite"));
  dummy.initialize();

  // Dragon Rush has 75% base accuracy
  const Move& move = pokedex_->move("dragon rush");
  FixType accuracy = move.getPrimaryAccuracy();

  // +1 accuracy boost multiplier is 4/3
  FixType boost = PokemonNonVolatile::aFV_base[FV_ACCURACY - 6][7];

  FixType probabilityToHit = boost * accuracy;
  FixType one(1.0);

  // The rounding error causes it to be slightly less than 1.0
  EXPECT_LT(probabilityToHit, one);
  EXPECT_EQ(probabilityToHit.intValue, one.intValue - 1);
}

/**
 * Triggers the assertion 'branchProbability > FixType(0)' in PkCUEngine::duplicateState
 * using the actual engine logic.
 *
 * Scenario:
 * 1. A speed tie split causes states to have 0.5 probability.
 * 2. A move with the discovered rounding error is used (accuracy ends up being 1.0 - epsilon).
 * 3. PkCUEngine tries to create a 'miss' branch with probability epsilon (1/2^27).
 * 4. The new branch probability is 0.5 * epsilon, which underflows to 0.
 * 5. The assertion fails.
 */
TEST_F(AccuracyReproTest, TriggerAssertionFailure) {
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("dragonite"))
        .addMove(pokedex_->move("dragon rush"))
        .setLevel(100));
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("dragonite"))
        .addMove(pokedex_->move("dragon rush"))
        .setLevel(100));

  // Create environment with speed tie enabled
  auto shared_nv = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
  engine_->setEnvironment(shared_nv);

  auto env_data = EnvironmentVolatileData::create(*shared_nv);
  auto env = EnvironmentVolatile(*shared_nv, env_data);

  // Set accuracy boost to +1 for Team A
  env.getTeam(TEAM_A).getPKV().setBoost(FV_ACCURACY, 1);

  Action actionA = Action::move(0); // Dragon Rush
  Action actionB = Action::move(0); // Dragon Rush

  // This expects the process to die due to the assertion failure.
  EXPECT_DEATH({
    engine_->updateState(env, actionA, actionB);
  }, "Assertion `branchProbability > FixType\\(0\\)' failed");
}
