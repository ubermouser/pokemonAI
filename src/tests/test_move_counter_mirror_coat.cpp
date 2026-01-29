#include "engine_test.hpp"


class CounterMirrorCoatTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    // Team A: [Blastoise (Slow), Mew (Fast)]
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blastoise"))
          .addMove(pokedex_->move("counter"))
          .addMove(pokedex_->move("mirror coat"))
          .addMove(pokedex_->move("substitute"))
          .setIV(FV_SPEED, 0)
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("counter"))
          .addMove(pokedex_->move("substitute"))
          .setIV(FV_SPEED, 31)
          .setLevel(100));

    // Team B: [Charmander (Fast), Gengar (Fast), Umbreon (Fast), Machamp
    // (Slow)]
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("cut"))
          .addMove(pokedex_->move("ember"))
          .addMove(pokedex_->move("will-o-wisp"))
          .addMove(pokedex_->move("counter"))
          .setIV(FV_SPEED, 31)
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("body slam"))
          .setIV(FV_SPEED, 31)
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("umbreon"))
          .addMove(pokedex_->move("dark pulse"))
          .setIV(FV_SPEED, 31)
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("machamp"))
          .addMove(pokedex_->move("counter"))
          .addMove(pokedex_->move("brick break"))
          .setIV(FV_SPEED, 0)
          .setLevel(100));
    // clang-format on

    env_nv = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(env_nv);

    both_hit = both_hit.team(0).hasHit().team(1).hasHit();

    // Standard state: Blastoise vs Charmander
    state_standard = PossibleEnvironments();
    state_standard.setNonvolatileEnvironment(env_nv);
    state_standard.push_back(
        EnvironmentPossibleData::create(engine_->initialState().data()));

    // Ghost state: Blastoise vs Gengar (Swap Charmander to Gengar)
    auto ghost_result = engine_->updateState(
        state_standard.where1().getEnv(), Action::wait(), Action::swap(1));
    state_ghost = ghost_result;

    // Dark state: Blastoise vs Umbreon (Swap Charmander to Umbreon)
    auto dark_result = engine_->updateState(
        state_standard.where1().getEnv(), Action::wait(), Action::swap(2));
    state_dark = dark_result;

    // Speed state: Mew vs Machamp (Swap Blastoise to Mew, Charmander to
    // Machamp)
    auto speed_setup = engine_->updateState(
        state_standard.where1().getEnv(), Action::swap(1), Action::swap(3));
    state_speed = speed_setup;

    // Substitute sequence: Mew (Fast) vs Machamp (Slow)
    // Turn 1: Mew uses Substitute, Machamp uses Cut
    sub_turn1 = engine_->updateState(
        state_speed.where1().getEnv(), Action::move(1), Action::move(1));
    // Turn 2: Machamp attacks substitute, Mew uses Counter
    sub_turn2 = engine_->updateState(
        sub_turn1.where1Hit(0).getEnv(), Action::move(1), Action::move(0));
  }

  EnvironmentBitfield both_hit;
  std::shared_ptr<const EnvironmentNonvolatile> env_nv;
  PossibleEnvironments state_standard;
  PossibleEnvironments state_ghost;
  PossibleEnvironments state_dark;
  PossibleEnvironments state_speed;
  PossibleEnvironments sub_turn1;
  PossibleEnvironments sub_turn2;
};


TEST_F(CounterMirrorCoatTest, CounterPhysical) {
  auto result = engine_->updateState(
      state_standard.where1().getEnv(), Action::move(0), Action::move(0));
  auto state = result.where1(both_hit);
  
  uint32_t damageToBlastoise = state.teammate(0, 0).getMissingHP();
  uint32_t damageToCharmander = state.teammate(1, 0).getMissingHP();

  EXPECT_GT(damageToBlastoise, 0);
  EXPECT_EQ(damageToCharmander, damageToBlastoise * 2);
}


TEST_F(CounterMirrorCoatTest, CounterFailsOnSpecial) {
  auto result = engine_->updateState(
      state_standard.where1().getEnv(), Action::move(0), Action::move(1));
  auto state = result.where1(both_hit);
  
  EXPECT_GT(state.teammate(0, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(1, 0).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, MirrorCoatSpecial) {
  auto result = engine_->updateState(
      state_standard.where1().getEnv(), Action::move(1), Action::move(1));
  auto state = result.where1(both_hit);
  
  uint32_t damageToBlastoise = state.teammate(0, 0).getMissingHP();
  uint32_t damageToCharmander = state.teammate(1, 0).getMissingHP();

  EXPECT_GT(damageToBlastoise, 0);
  EXPECT_EQ(damageToCharmander, damageToBlastoise * 2);
}


TEST_F(CounterMirrorCoatTest, MirrorCoatFailsOnPhysical) {
  auto result = engine_->updateState(
      state_standard.where1().getEnv(), Action::move(1), Action::move(0));
  auto state = result.where1(both_hit);
  
  EXPECT_GT(state.teammate(0, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(1, 0).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, CounterFailsIfMovingFirst) {
  // Mew uses Counter, Machamp uses Counter. Mew is faster.
  auto result = engine_->updateState(
      state_speed.where1().getEnv(), Action::move(0), Action::move(0));
  auto state = result.where1Hit(0);

  EXPECT_EQ(state.teammate(1, 3).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, CounterFailsOnStatus) {
  auto result = engine_->updateState(
      state_standard.where1().getEnv(), Action::move(0), Action::move(2));
  auto state = result.where1(both_hit);
  
  EXPECT_EQ(state.teammate(1, 0).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, CounterImmuneGhost) {
  auto result = engine_->updateState(
      state_ghost.where1(EnvironmentBitfield().team(1).hasSwitched()).getEnv(),
      Action::move(0),
      Action::move(0));
  auto state = result.where1(both_hit);

  EXPECT_EQ(state.teammate(1, 1).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, MirrorCoatImmuneDark) {
  auto result = engine_->updateState(
      state_dark.where1(EnvironmentBitfield().team(1).hasSwitched()).getEnv(),
      Action::move(1),
      Action::move(0));
  auto state = result.where1(both_hit);

  EXPECT_EQ(state.teammate(1, 2).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, CounterFailsIfDamageAbsorbedBySubstitute) {
  auto state1 = sub_turn1.where1Hit(0);
  auto state2 = sub_turn2.where1Hit(1);

  // Mew should have missingHP from Turn 1 (Substitute cost), but 0 change in
  // Turn 2.
  uint32_t hp1 = state1.teammate(0, 1).getHP();
  uint32_t hp2 = state2.teammate(0, 1).getHP();

  EXPECT_EQ(hp1, hp2); // No damage taken in Turn 2
  EXPECT_EQ(state2.teammate(1, 3).getMissingHP(), 0);  // Counter should fail
}
