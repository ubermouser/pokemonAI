#include "engine_test.hpp"


class RampageTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    // Team A: [Smeargle (Rampager), Pikachu (Backup)]
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("outrage"))
          .addMove(pokedex_->move("petal dance"))
          .addMove(pokedex_->move("thrash"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("pikachu"))
          .setLevel(100));

    // Team B: [Metagross (Neutral), Gengar (Ghost-type)]
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("metagross"))
          .addMove(pokedex_->move("confusion"))  // a damaging move that might confuse
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .setLevel(100));
    // clang-format on

    env_nv =
        std::make_shared<const EnvironmentNonvolatile>(team_a, team_b, true);
    engine_->setEnvironment(env_nv);
  }

  PossibleEnvironments setupRampage(int moveIdx) {
    // Start rampage move on Turn 1
    return engine_->updateState(
        engine_->initialState(), Action::move(moveIdx), Action::wait());
  }

  PossibleEnvironments advanceRampage(
      const ConstEnvironmentVolatile& env, int moveIdx) {
    // Continue locked-in rampage move
    return engine_->updateState(env, Action::move(moveIdx), Action::wait());
  }

  PossibleEnvironments setupRampage3Turns(int moveIdx) {
    auto turn1 = setupRampage(moveIdx);
    auto turn2 = advanceRampage(
        turn1
            .where1([](const ConstEnvironmentPossible& p) {
              return p.teammate(TEAM_A, 0).status().cTeammate.lockIn_duration ==
                     2;
            })
            .getEnv(),
        moveIdx);
    auto state2 = turn2.where1([](const ConstEnvironmentPossible& p) {
      return p.teammate(TEAM_A, 0).status().cTeammate.lockIn_duration == 1;
    });
    return advanceRampage(state2.getEnv(), moveIdx);
  }

  PossibleEnvironments setupRampageVsGhost(int moveIdx) {
    auto swap = engine_->updateState(
        engine_->initialState(), Action::wait(), Action::swap(1));
    return engine_->updateState(
        swap.where1Switch(TEAM_B).getEnv(),
        Action::move(moveIdx),
        Action::wait());
  }

  PossibleEnvironments setupRampageParalyzed(int moveIdx) {
    auto setup = engine_->updateState(
        engine_->initialState(), Action::wait(), Action::wait());
    auto state0 = setup.where1();
    state0.teammate(TEAM_A, 0).setStatusAilment(AIL_NV_PARALYSIS);
    auto turn1 = engine_->updateState(
        state0.getEnv(), Action::move(moveIdx), Action::wait());
    auto state1 = turn1.where1();
    return engine_->updateState(
        state1.getEnv(), Action::move(moveIdx), Action::wait());
  }

  PossibleEnvironments setupRampageInterruptedByDeath(int moveIdx) {
    // Setup: Smeargle at 1 HP, Metagross moves first and kills with Confusion
    // (or equivalent)
    auto setup = engine_->updateState(
        engine_->initialState(), Action::wait(), Action::wait());
    auto state0 = setup.where1();
    state0.teammate(TEAM_A, 0).setHP(1);

    // Turn 1: Smeargle uses Outrage. Metagross waits.
    auto turn1 = engine_->updateState(
        state0.getEnv(), Action::move(moveIdx), Action::wait());
    auto state1 = turn1.where1();

    // Turn 2: Metagross uses Tackle (move 0?), Smeargle uses Outrage (locked
    // in) We need Metagross to be faster.
    state1.teammate(TEAM_B, 0).status().cTeammate.boosts.B_SPE = 6;

    return engine_->updateState(
        state1.getEnv(), Action::move(moveIdx), Action::move(0));
  }

  std::shared_ptr<const EnvironmentNonvolatile> env_nv;
};


TEST_F(RampageTest, OutrageLockIn) {
  auto turn1 = setupRampage(0);
  auto state = turn1.where1().getEnv();

  // The pokemon cannot switch out or perform other moves when rampaging:
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::move(0)));
  EXPECT_FALSE(
      engine_->isValidAction(state, Actor(TEAM_A, 0), Action::move(1)));
  EXPECT_FALSE(
      engine_->isValidAction(state, Actor(TEAM_A, 0), Action::move(2)));
  EXPECT_FALSE(
      engine_->isValidAction(state, Actor(TEAM_A, 0), Action::move(3)));
  EXPECT_FALSE(
      engine_->isValidAction(state, Actor(TEAM_A, 0), Action::swap(1)));

  // lockIn_duration should be > 0 (it was set to 3 at start of turn, then
  // decremented to 2 at end of turn)
  EXPECT_EQ(state.teammate(TEAM_A, 0).status().cTeammate.lockIn_duration, 2);
}


TEST_F(RampageTest, OutrageConfusion) {
  auto result = setupRampage3Turns(0);
  auto state3 = result.where1();

  EXPECT_EQ(state3.teammate(TEAM_A, 0).status().cTeammate.lockIn_duration, 0);
  EXPECT_EQ(
      state3.teammate(TEAM_A, 0).status().cTeammate.confused,
      AIL_V_CONFUSED_5T);

  // Now other moves are valid again
  EXPECT_TRUE(engine_->isValidAction(
      state3.getEnv(), Actor(TEAM_A, 0), Action::move(1)));
  EXPECT_TRUE(engine_->isValidAction(
      state3.getEnv(), Actor(TEAM_A, 0), Action::swap(1)));
}


TEST_F(RampageTest, ThrashInterruptedByImmunity) {
  auto result = setupRampageVsGhost(2);
  auto state1 = result.where1();

  // In Gen 4: If a rampage move hits an immune target, it should end
  // immediately and NOT cause confusion.
  EXPECT_EQ(state1.teammate(TEAM_A, 0).status().cTeammate.lockIn_duration, 0);
  EXPECT_EQ(state1.teammate(TEAM_A, 0).status().cTeammate.confused, 0);
}


TEST_F(RampageTest, PetalDanceMinimal) {
  auto turn1 = setupRampage(1);
  auto state = turn1.where1().getEnv();
  EXPECT_EQ(state.teammate(TEAM_A, 0).status().cTeammate.lockIn_duration, 2);
}


TEST_F(RampageTest, ThrashMinimal) {
  auto turn1 = setupRampage(2);
  auto state = turn1.where1().getEnv();
  EXPECT_EQ(state.teammate(TEAM_A, 0).status().cTeammate.lockIn_duration, 2);
}


TEST_F(RampageTest, OutrageInterruptedByParalysis) {
  auto result = setupRampageParalyzed(0);
  auto state2 = result.where1Miss(TEAM_A);

  // NOTE: If the engine currently doesn't implement lock-in clearing on
  // paralysis, this will reveal it.
  EXPECT_EQ(state2.teammate(TEAM_A, 0).status().cTeammate.lockIn_duration, 0);
  EXPECT_EQ(state2.teammate(TEAM_A, 0).status().cTeammate.confused, 0);
}


TEST_F(RampageTest, OutrageInterruptedbyFaint) {
  auto result = setupRampageInterruptedByDeath(0);
  auto state2 = result.where1Hit(TEAM_B);
  auto smeargle = state2.teammate(TEAM_A, 0);

  // The pokemon should be fainted
  EXPECT_FALSE(smeargle.isAlive());

  // Rampage should be cleared
  EXPECT_EQ(smeargle.status().cTeammate.lockIn_duration, 0);
  EXPECT_EQ(smeargle.status().cTeammate.lockIn_action, 0);
}


TEST_F(RampageTest, OutrageDoesNotPreventFreeSwitch) {
  auto result = setupRampageInterruptedByDeath(0);
  auto state2 = result.where1Hit(TEAM_B);

  // The fainted pokemon should not be prevented from switching (re-replacement)
  EXPECT_TRUE(engine_->isValidAction(
      state2.getEnv(), Actor(TEAM_A, 0), Action::swap(1)));

  // The fainted pokemon should not be able to move
  EXPECT_FALSE(engine_->isValidAction(
      state2.getEnv(), Actor(TEAM_A, 0), Action::move(0)));
}
