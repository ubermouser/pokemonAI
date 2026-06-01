#include <fmt/ranges.h>

#include "engine_test.hpp"


class Gen4BasicEngineTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("smeargle"))
            .addMove(pokedex_->move("mud-slap"))
            .addMove(pokedex_->move("sweet scent"))
            .addMove(pokedex_->move("swords dance"))
            .addMove(pokedex_->move("screech")))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("aipom"))
            .addMove(pokedex_->move("screech")));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("smeargle"))
            .addMove(pokedex_->move("mud-slap"))
            .addMove(pokedex_->move("sweet scent"))
            .addMove(pokedex_->move("glare"))
            .addMove(pokedex_->move("screech")))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("squirtle"))
            .addMove(pokedex_->move("tail whip")));
    // clang-format on
    auto environment = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment);
  }

  PossibleEnvironments runMudSlapSweetScentTurn(const ConstEnvironmentVolatile& state) {
    return engine_->updateState(state, Action::move(0), Action::move(1));
  }

  PossibleEnvironments runTurn1Wait() {
    return engine_->updateState(engine_->initialState(), Action::move(2), Action::wait());
  }

  PossibleEnvironments runTurn2Switch(const ConstEnvironmentVolatile& state) {
    return engine_->updateState(state, Action::swap(1), Action::wait());
  }
};


TEST_F(Gen4BasicEngineTest, SwordsDanceBuffsAttack) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      Action::move(2),  // swords dance
      Action::wait());

  result.printStates();
  // swords dance has --- P.Accuracy and 100 S.Accuracy, so it should always hit
  // and status.
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result.where1().flagsFor(TEAM_A).isHit(), true);
  EXPECT_EQ(result.where1().flagsFor(TEAM_A).isSecondary(), true);
  EXPECT_EQ(result.where1().teammate(TEAM_A, 0).getBoost(FV_ATTACK), 2);
}


TEST_F(Gen4BasicEngineTest, ScreechDebuffsDefenseOnHit) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(3), Action::wait());

  result.printStates();
  // screech has 85 S.Accuracy, so it can hit or miss.
  // It has --- P.Accuracy.
  EXPECT_EQ(result.size(), 2);

  auto hitState = result.where1Status(TEAM_A);
  EXPECT_EQ(hitState.flagsFor(TEAM_A).isHit(), true);
  EXPECT_EQ(hitState.flagsFor(TEAM_A).isSecondary(), true);
  EXPECT_EQ(hitState.teammate(TEAM_B, 0).getBoost(FV_DEFENSE), -2);
}


TEST_F(Gen4BasicEngineTest, ScreechNoEffectOnMiss) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(3), Action::wait());

  result.printStates();
  EXPECT_EQ(result.size(), 2);

  auto missState = result.where1HitNoStatus(TEAM_A);
  EXPECT_EQ(missState.flagsFor(TEAM_A).isHit(), true);
  EXPECT_EQ(missState.flagsFor(TEAM_A).isSecondary(), false);
  EXPECT_EQ(missState.teammate(TEAM_B, 0).getBoost(FV_DEFENSE), 0);
}


TEST_F(Gen4BasicEngineTest, GlareParalyzesOnHit) {
  // Team B uses glare (Move 2)
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::wait(), Action::move(2));

  result.printStates();
  // Glare has 75% accuracy and no damage/crit.
  // It should produce two states: hit and miss.
  EXPECT_EQ(result.size(), 2);

  auto hit_state = result.where1Status(TEAM_B);
  EXPECT_EQ(
      hit_state.teammate(TEAM_A, 0).getStatusAilment(), AIL_NV_PARALYSIS);
}


TEST_F(Gen4BasicEngineTest, GlareNoEffectOnMiss) {
  // Team B uses glare (Move 2)
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::wait(), Action::move(2));

  result.printStates();
  EXPECT_EQ(result.size(), 2);

  auto miss_state = result.where1HitNoStatus(TEAM_B);
  EXPECT_EQ(miss_state.teammate(TEAM_A, 0).getStatusAilment(), AIL_NV_NONE);
}


TEST_F(Gen4BasicEngineTest, HighEvasionAndAccuracy) {
  spdlog::set_level(spdlog::level::warn);
  // Reproduce branchProbability > FixType(0) assertion failure
  engine_->setAccuracy(16);

  PossibleEnvironments results = runMudSlapSweetScentTurn(engine_->initialState());
  auto currentState = results.where1Hit(TEAM_A);
  results.printStates();

  // Run 7 turns of accuracy/evasion debuffs
  for (int turn = 1; turn <= 7; ++turn) {
    std::cout << "Turn " << turn << ": Mud-slap vs Sweet Scent" << std::endl;
    // We want to ensure Mud-slap hits to keep the debuff loop going.
    results = runMudSlapSweetScentTurn(currentState);
    results.printStates();

    ASSERT_FALSE(results.empty())
        << "Engine returned no states on turn " << turn;

    // will throw if Mud-slap fails to hit
    currentState = results.where1Hit(TEAM_A);
  }

  std::cout << "Successfully completed 7 turns of Mud-slap vs Sweet Scent"
            << std::endl;
}


TEST_F(Gen4BasicEngineTest, InitialNumRoundsInPlayIsZero) {
  auto state0 = engine_->initialState();
  EXPECT_EQ(state0.teammate(TEAM_A, 0).status().numRoundsInPlay, 0);
  EXPECT_EQ(state0.teammate(TEAM_B, 0).status().numRoundsInPlay, 0);
}


TEST_F(Gen4BasicEngineTest, NumRoundsInPlayIncrementsEachTurn) {
  auto turn1 = runTurn1Wait();
  auto state1 = turn1.where1();
  EXPECT_EQ(state1.teammate(TEAM_A, 0).status().numRoundsInPlay, 1);
  EXPECT_EQ(state1.teammate(TEAM_B, 0).status().numRoundsInPlay, 1);
}


TEST_F(Gen4BasicEngineTest, NumRoundsInPlayResetsOnSwitch) {
  auto turn1 = runTurn1Wait();
  auto turn2 = runTurn2Switch(turn1.where1());
  auto state2 = turn2.where1();

  // Aipom (now active) has numRoundsInPlay = 0 because it just switched in.
  EXPECT_TRUE(state2.teammate(TEAM_A, 1).isActive());
  EXPECT_EQ(state2.teammate(TEAM_A, 1).status().numRoundsInPlay, 0);

  // Smeargle is inactive now.
  EXPECT_FALSE(state2.teammate(TEAM_A, 0).isActive());

  // TEAM_B Smeargle stayed in, so its numRoundsInPlay should increment to 2.
  EXPECT_EQ(state2.teammate(TEAM_B, 0).status().numRoundsInPlay, 2);
}
