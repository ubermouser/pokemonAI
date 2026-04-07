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
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("ambipom"))
          .addMove(pokedex_->move("u-turn"))
          .setIV(FV_SPEED, 31)
          .setLevel(100));
    // clang-format on

    env_nv = std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(env_nv);

    both_hit = EnvironmentBitfield().flagsFor(TEAM_A).setHit().flagsFor(TEAM_B).setHit();
  }

  PossibleEnvironments setupStandard() {
    // Standard state: Blastoise vs Charmander
    PossibleEnvironments state;
    state.setNonvolatileEnvironment(env_nv);
    state.push_back(
        EnvironmentPossibleData::create(engine_->initialState().data()));
    return state;
  }

  PossibleEnvironments setupGhost() {
    // Ghost state: Blastoise vs Gengar (Swap Charmander to Gengar)
    return engine_->updateState(
        setupStandard().where1().getEnv(), Action::wait(), Action::swap(1));
  }

  PossibleEnvironments setupDark() {
    // Dark state: Blastoise vs Umbreon (Swap Charmander to Umbreon)
    return engine_->updateState(
        setupStandard().where1().getEnv(), Action::wait(), Action::swap(2));
  }

  PossibleEnvironments setupSpeed() {
    // Speed state: Mew vs Machamp (Swap Blastoise to Mew, Charmander to
    // Machamp)
    return engine_->updateState(
        setupStandard().where1().getEnv(), Action::swap(1), Action::swap(3));
  }

  PossibleEnvironments setupSubTurn1() {
    // Substitute sequence: Mew (Fast) vs Machamp (Slow)
    // Turn 1: Mew uses Substitute, Machamp uses Cut
    return engine_->updateState(
        setupSpeed().where1().getEnv(), Action::moveAlly(1, 1), Action::move(1));
  }

  PossibleEnvironments setupSubTurn2() {
    // Turn 2: Machamp attacks substitute, Mew uses Counter
    return engine_->updateState(
        setupSubTurn1().where1Hit(0).getEnv(), Action::moveAlly(1, 1), Action::move(0));
  }

  PossibleEnvironments setupUTurnTurn1() {
    // U-turn sequence: Blastoise (Slow) vs Ambipom (Fast)
    // Turn 1: Swap Charmander to Ambipom (index 4)
    return engine_->updateState(
        setupStandard().where1().getEnv(), Action::wait(), Action::swap(4));
  }

  PossibleEnvironments setupUTurnTurn2() {
    // Turn 2: Ambipom uses U-turn, Blastoise uses Counter
    // Ambipom switches to Machamp (index 3)
    return engine_->updateState(
        setupUTurnTurn1()
            .where1(EnvironmentBitfield().flagsFor(TEAM_B).setSwitched())
            .getEnv(),
        Action::move(0), Action::moveAlly(0, 3));
  }

  EnvironmentBitfield both_hit;
  std::shared_ptr<const EnvironmentNonvolatile> env_nv;
};


TEST_F(CounterMirrorCoatTest, CounterPhysical) {
  auto standard = setupStandard();
  auto result = engine_->updateState(
      standard.where1().getEnv(), Action::move(0), Action::move(0));
  auto state = result.where1(both_hit);
  
  uint32_t damageToBlastoise = state.teammate(0, 0).getMissingHP();
  uint32_t damageToCharmander = state.teammate(1, 0).getMissingHP();

  EXPECT_GT(damageToBlastoise, 0);
  EXPECT_EQ(damageToCharmander, damageToBlastoise * 2);
}


TEST_F(CounterMirrorCoatTest, CounterFailsOnSpecial) {
  auto standard = setupStandard();
  auto result = engine_->updateState(
      standard.where1().getEnv(), Action::move(0), Action::move(1));
  auto state = result.where1(both_hit);
  
  EXPECT_GT(state.teammate(0, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(1, 0).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, MirrorCoatSpecial) {
  auto standard = setupStandard();
  auto result = engine_->updateState(
      standard.where1().getEnv(), Action::move(1), Action::move(1));
  auto state = result.where1(both_hit);
  
  uint32_t damageToBlastoise = state.teammate(0, 0).getMissingHP();
  uint32_t damageToCharmander = state.teammate(1, 0).getMissingHP();

  EXPECT_GT(damageToBlastoise, 0);
  EXPECT_EQ(damageToCharmander, damageToBlastoise * 2);
}


TEST_F(CounterMirrorCoatTest, MirrorCoatFailsOnPhysical) {
  auto standard = setupStandard();
  auto result = engine_->updateState(
      standard.where1().getEnv(), Action::move(1), Action::move(0));
  auto state = result.where1(both_hit);
  
  EXPECT_GT(state.teammate(0, 0).getMissingHP(), 0);
  EXPECT_EQ(state.teammate(1, 0).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, CounterFailsIfMovingFirst) {
  // Mew uses Counter, Machamp uses Counter. Mew is faster.
  auto speed = setupSpeed();
  auto result = engine_->updateState(
      speed.where1().getEnv(), Action::move(0), Action::move(0));
  auto state = result.where1Hit(0);

  EXPECT_EQ(state.teammate(1, 3).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, CounterFailsOnStatus) {
  auto standard = setupStandard();
  auto result = engine_->updateState(
      standard.where1().getEnv(), Action::move(0), Action::move(2));
  auto state = result.where1(both_hit);
  
  EXPECT_EQ(state.teammate(1, 0).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, CounterImmuneGhost) {
  auto ghost = setupGhost();
  auto result = engine_->updateState(
      ghost.where1(EnvironmentBitfield().flagsFor(TEAM_B).setSwitched()).getEnv(),
      Action::move(0),
      Action::move(0));
  auto state = result.where1(both_hit);

  EXPECT_EQ(state.teammate(1, 1).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, MirrorCoatImmuneDark) {
  auto dark = setupDark();
  auto result = engine_->updateState(
      dark.where1(EnvironmentBitfield().flagsFor(TEAM_B).setSwitched()).getEnv(),
      Action::move(1),
      Action::move(0));
  auto state = result.where1(both_hit);

  EXPECT_EQ(state.teammate(1, 2).getMissingHP(), 0);
}


TEST_F(CounterMirrorCoatTest, CounterFailsIfDamageAbsorbedBySubstitute) {
  auto sub1 = setupSubTurn1();
  auto state1 = sub1.where1Hit(0);
  auto sub2 = setupSubTurn2();
  auto state2 = sub2.where1Hit(1);

  // Mew should have missingHP from Turn 1 (Substitute cost), but 0 change in
  // Turn 2.
  uint32_t hp1 = state1.teammate(0, 1).getHP();
  uint32_t hp2 = state2.teammate(0, 1).getHP();

  EXPECT_EQ(hp1, hp2); // No damage taken in Turn 2
  EXPECT_EQ(state2.teammate(1, 3).getMissingHP(), 0);  // Counter should fail
}


TEST_F(CounterMirrorCoatTest, CounterUTurnSwitch) {
  auto uturn2 = setupUTurnTurn2();
  auto state = uturn2.where1(both_hit);

  uint32_t damageToBlastoise = state.teammate(0, 0).getMissingHP();
  uint32_t damageToMachamp = state.teammate(1, 3).getMissingHP();

  EXPECT_GT(damageToBlastoise, 0);
  EXPECT_EQ(damageToMachamp, damageToBlastoise * 2);
}
