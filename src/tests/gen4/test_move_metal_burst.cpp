#include "engine_test.hpp"

class MetalBurstTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // Team A: [Aggron (Slow)]
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("aggron"))
          .setInitialItem(pokedex_->item("focus sash"))
          .addMove(pokedex_->move("metal burst"))
          .setIV(FV_SPEED, 0)
          .setLevel(100));

    // Team B: [Garchomp (Fast, Physical, Status), Starmie (Fast, Special), Shuckle (Slow, Status)]
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("garchomp"))
          .addMove(pokedex_->move("earthquake"))
          .addMove(pokedex_->move("swords dance"))
          .setIV(FV_SPEED, 31)
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("starmie"))
          .addMove(pokedex_->move("surf"))
          .setIV(FV_SPEED, 31)
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("shuckle"))
          .addMove(pokedex_->move("toxic"))
          .setIV(FV_SPEED, 0)
          .setLevel(100));

    env_nv = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(env_nv);

    both_hit = both_hit.team(0).hasHit().team(1).hasHit();

    // Standard state: Aggron vs Garchomp
    state_physical = PossibleEnvironments();
    state_physical.setNonvolatileEnvironment(env_nv);
    state_physical.push_back(
        EnvironmentPossibleData::create(engine_->initialState().data()));

    // Special state: Aggron vs Starmie (Swap Garchomp to Starmie)
    auto special_result = engine_->updateState(
        state_physical.where1().getEnv(), Action::wait(), Action::swap(1));
    state_special = special_result;

    // Slow state: Aggron vs Shuckle (Swap Garchomp to Shuckle)
    auto slow_result = engine_->updateState(
        state_physical.where1().getEnv(), Action::wait(), Action::swap(2));
    state_slow = slow_result;
  }

  EnvironmentBitfield both_hit;
  std::shared_ptr<const EnvironmentNonvolatile> env_nv;
  PossibleEnvironments state_physical;
  PossibleEnvironments state_special;
  PossibleEnvironments state_slow;
};

TEST_F(MetalBurstTest, MetalBurstPhysical) {
  // Aggron: Metal Burst (0), Garchomp: Earthquake (0)
  auto result = engine_->updateState(
      state_physical.where1().getEnv(), Action::move(0), Action::move(0));
  auto state = result.where1(both_hit);

  uint32_t damageToAggron = state.teammate(0, 0).getMissingHP();
  uint32_t damageToGarchomp = state.teammate(1, 0).getMissingHP();

  EXPECT_GT(damageToAggron, 0);

  uint32_t expectedDamage = damageToAggron * 1.5;
  uint32_t garchompHP = state.teammate(1, 0).getHP();
  uint32_t garchompMaxHP = garchompHP + damageToGarchomp;

  if (expectedDamage >= garchompMaxHP) {
    EXPECT_EQ(damageToGarchomp, garchompMaxHP);
  } else {
    EXPECT_EQ(damageToGarchomp, expectedDamage);
  }
}

TEST_F(MetalBurstTest, MetalBurstSpecial) {
  // Starmie uses Surf (0), Aggron uses Metal Burst (0)
  auto result = engine_->updateState(
      state_special.where1(EnvironmentBitfield().team(1).hasSwitched()).getEnv(),
      Action::move(0),
      Action::move(0));
  auto state = result.where1(both_hit);

  uint32_t damageToAggron = state.teammate(0, 0).getMissingHP();
  uint32_t damageToStarmie = state.teammate(1, 1).getMissingHP();

  EXPECT_GT(damageToAggron, 0);
  uint32_t expectedDamage = damageToAggron * 1.5;
  uint32_t starmieHP = state.teammate(1, 1).getHP();
  uint32_t starmieMaxHP = starmieHP + damageToStarmie;

  if (expectedDamage >= starmieMaxHP) {
    EXPECT_EQ(damageToStarmie, starmieMaxHP);
  } else {
    EXPECT_EQ(damageToStarmie, expectedDamage);
  }
}

TEST_F(MetalBurstTest, MetalBurstFailsIfMovingFirst) {
  // Aggron vs Shuckle. Aggron is faster.
  // Aggron uses Metal Burst (0), Shuckle uses Toxic (0).
  auto result = engine_->updateState(
      state_slow.where1(EnvironmentBitfield().team(1).hasSwitched()).getEnv(),
      Action::move(0),
      Action::move(0));

  // Aggron moves first -> Fails.
  // Shuckle uses Toxic.

  // We check that Shuckle took no damage.
  // Using where1Hit(1) because Shuckle hits with Toxic.
  // Aggron might not "hit" because it failed.

  // Actually, let's just inspect the first result.
  auto state = result.at(0);

  EXPECT_EQ(state.teammate(1, 2).getMissingHP(), 0);
}

TEST_F(MetalBurstTest, MetalBurstFailsOnStatus) {
  // Aggron vs Garchomp. Garchomp uses Swords Dance (1).
  // Aggron uses Metal Burst (0).
  auto result = engine_->updateState(
      state_physical.where1().getEnv(), Action::move(0), Action::move(1));

  auto state = result.where1(both_hit);

  // Aggron should have taken no damage (Swords Dance).
  EXPECT_EQ(state.teammate(0, 0).getMissingHP(), 0);

  // Garchomp should have taken no damage (Metal Burst failed).
  EXPECT_EQ(state.teammate(1, 0).getMissingHP(), 0);
}
