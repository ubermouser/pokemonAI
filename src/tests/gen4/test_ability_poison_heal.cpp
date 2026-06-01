#include "engine_test.hpp"

class PoisonHealTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    // Team A: Shroomish with Poison Heal (teammate 0) and Squirtle with Torrent (teammate 1)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("shroomish"))
          .setAbility(pokedex_->ability("poison heal"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle"))
          .setAbility(pokedex_->ability("torrent"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    // Team B: Smeargle
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("seismic toss"))  // Deal 100 damage (Move 0)
          .addMove(pokedex_->move("poisonpowder"))  // Poison status (Move 1)
          .addMove(pokedex_->move("toxic"))         // Toxic status (Move 2)
          .addMove(pokedex_->move("splash"))        // Dummy (Move 3)
          .setLevel(100));
    // clang-format on

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  // Turn 1: Smeargle uses Seismic Toss (Move 0) to damage Shroomish.
  // Shroomish uses Tackle.
  PossibleEnvironments setupDamagedShroomish() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::move(0));
  }

  // Turn 2: Smeargle uses Poison Powder (Move 1) to poison Shroomish.
  // Shroomish waits.
  PossibleEnvironments setupPoisonedShroomish(
      const ConstEnvironmentVolatile& state) {
    return engine_->updateState(state, Action::wait(), Action::move(1));
  }

  // Turn 2: Smeargle uses Toxic (Move 2) to toxic-poison Shroomish.
  // Shroomish waits.
  PossibleEnvironments setupToxicPoisonedShroomish(
      const ConstEnvironmentVolatile& state) {
    return engine_->updateState(state, Action::wait(), Action::move(2));
  }

  // Turn 1: Team A swaps Shroomish out for Squirtle (index 1).
  // Team B (Smeargle) uses Seismic Toss (Move 0) to damage Squirtle.
  PossibleEnvironments setupDamagedSquirtle() {
    return engine_->updateState(
        engine_->initialState(), Action::swap(1), Action::move(0));
  }

  // Turn 2: Team A waits (Squirtle is active).
  // Team B uses Poison Powder (Move 1) to poison Squirtle.
  PossibleEnvironments setupPoisonedSquirtle(
      const ConstEnvironmentVolatile& state) {
    return engine_->updateState(state, Action::wait(), Action::move(1));
  }
};


TEST_F(PoisonHealTest, PoisonHealRestoresHPWhenPoisoned) {
  // Step 1: Damage Shroomish
  auto turn1 = setupDamagedShroomish();
  auto state1 = turn1.where1();
  double hp_percent_before = state1.teammate(TEAM_A, 0).getPercentHP();

  // Step 2: Poison Shroomish
  auto turn2 = setupPoisonedShroomish(state1.getEnv());
  // Find the state where Shroomish is poisoned
  auto state2 = turn2.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(TEAM_A, 0).getStatusAilment() == AIL_NV_POISON;
  });

  double hp_percent_after = state2.teammate(TEAM_A, 0).getPercentHP();

  // Poison Heal restores 1/8th (12.5%) of Max HP at the end of the turn
  EXPECT_NEAR(hp_percent_after, hp_percent_before + 0.125, 0.005);
}


TEST_F(PoisonHealTest, PoisonHealRestoresHPWhenToxicPoisoned) {
  // Step 1: Damage Shroomish
  auto turn1 = setupDamagedShroomish();
  auto state1 = turn1.where1();
  double hp_percent_before = state1.teammate(TEAM_A, 0).getPercentHP();

  // Step 2: Toxic Shroomish
  auto turn2 = setupToxicPoisonedShroomish(state1.getEnv());
  // Find the state where Shroomish is toxic-poisoned
  auto state2 = turn2.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(TEAM_A, 0).getStatusAilment() == AIL_NV_POISON_TOXIC;
  });

  double hp_percent_after = state2.teammate(TEAM_A, 0).getPercentHP();

  // Poison Heal restores 1/8th (12.5%) of Max HP at the end of the turn
  EXPECT_NEAR(hp_percent_after, hp_percent_before + 0.125, 0.005);
}


TEST_F(PoisonHealTest, PoisonHealPreventsToxicCounterIncrease) {
  // Step 1: Damage Shroomish
  auto turn1 = setupDamagedShroomish();
  auto state1 = turn1.where1();

  // Step 2: Toxic Shroomish
  auto turn2 = setupToxicPoisonedShroomish(state1.getEnv());
  auto state2 = turn2.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(TEAM_A, 0).getStatusAilment() == AIL_NV_POISON_TOXIC;
  });

  // Check initial toxic tier is 0
  EXPECT_EQ(state2.teammate(TEAM_A, 0).status().toxicPoison_tier, 0);

  // Step 3: Wait another turn
  auto turn3 =
      engine_->updateState(state2.getEnv(), Action::wait(), Action::wait());
  auto state3 = turn3.where1();

  // Toxic tier should remain 0
  EXPECT_EQ(state3.teammate(TEAM_A, 0).status().toxicPoison_tier, 0);
}


TEST_F(PoisonHealTest, NormalPoisonDamageWithoutAbility) {
  // Step 1: Damage Squirtle
  auto turn1 = setupDamagedSquirtle();
  auto state1 = turn1.where1();
  double hp_percent_before = state1.teammate(TEAM_A, 1).getPercentHP();

  // Step 2: Poison Squirtle
  auto turn2 = setupPoisonedSquirtle(state1.getEnv());
  auto state2 = turn2.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(TEAM_A, 1).getStatusAilment() == AIL_NV_POISON;
  });

  double hp_percent_after = state2.teammate(TEAM_A, 1).getPercentHP();

  // Without Poison Heal, poison inflicts 1/8th (12.5%) of Max HP as damage
  EXPECT_NEAR(hp_percent_after, hp_percent_before - 0.125, 0.005);
}
