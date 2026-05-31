#include "mock_engine_test.hpp"


class BasicEngineTest : public MockEngineTest {
 protected:
  void SetUp() override {
    MockEngineTest::SetUp();
    plugin_calls.fill(0);

    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .addMove(pokedex_->move("move_any_adjacent"))            // index 0
            .addMove(pokedex_->move("move_any_adjacent_secondary"))  // index 1
            .addMove(pokedex_->move("move_self_buff"))               // index 2
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon2"))
            .addMove(pokedex_->move("move_any_adjacent_debuff"))   // index 0
            .addMove(pokedex_->move("move_faint"))                 // index 1
            .setLevel(100));
    // clang-format on
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);
  }
};


TEST_F(BasicEngineTest, PrimaryHitAndCrit) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  result.printStates();
  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.where1Hit(0).flagsFor(TEAM_A).isHit(), true);
  EXPECT_EQ(result.where1Miss(0).flagsFor(TEAM_A).isHit(), false);
  EXPECT_EQ(result.where1Crit(0).flagsFor(TEAM_A).isCrit(), true);

  EXPECT_GT(result.where1Hit(0).teammate(TEAM_B, 0).getMissingHP(), 0);
}


TEST_F(BasicEngineTest, PrimaryHitAgainstSwap) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::swap(1));

  result.printStates();
  auto state = result.where1Hit(0);
  // swapped-out pokemon takes no damage
  EXPECT_EQ(state.teammate(TEAM_B, 0).getMissingHP(), 0);
  // swapped-in pokemon takes damage
  EXPECT_GT(state.teammate(TEAM_B, 1).getMissingHP(), 0);
  // swapped-in pokemon is active
  EXPECT_TRUE(state.teammate(TEAM_B, 1).isActive());
  EXPECT_FALSE(state.teammate(TEAM_B, 0).isActive());
}


TEST_F(BasicEngineTest, Swap) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::swap(1), Action::wait());

  result.printStates();
  auto state = result.where1Switch(0);
  // swapped-in pokemon is active
  EXPECT_TRUE(state.teammate(TEAM_A, 1).isActive());
  EXPECT_FALSE(state.teammate(TEAM_A, 0).isActive());
}


TEST_F(BasicEngineTest, PrimaryHitStatusAndCrit) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  result.printStates();
  EXPECT_EQ(result.size(), 5);
  EXPECT_EQ(result.whereMiss(0).size(), 1);
  EXPECT_EQ(result.whereHit(0).size(), 4);
  EXPECT_EQ(result.whereHitNoCrit(0).size(), 2);
  EXPECT_EQ(result.whereHitNoStatus(0).size(), 2);
  EXPECT_EQ(result.whereCrit(0).size(), 2);
  EXPECT_EQ(result.whereStatus(0).size(), 2);
}


TEST_F(BasicEngineTest, ZeroesStatusUponFainting) {
  auto setupSwap = engine_->updateState(
      engine_->initialState(), Action::move(2), Action::swap(1));
  auto firstPkmnFaints = engine_->updateState(
      setupSwap.where1(), Action::move(2), Action::move(1));

  firstPkmnFaints.printStates();
  // speed tie: first pokemon may go before being fainted
  EXPECT_EQ(firstPkmnFaints.size(), 2);
  // both states collapse into one due to zeroing status
  EXPECT_EQ(firstPkmnFaints.getNumUnique(), 1);
  EXPECT_EQ(setupSwap.where1().teammate(TEAM_A, 0).getBoost(FV_ATTACK), 1);
  EXPECT_EQ(
      firstPkmnFaints.where1().teammate(TEAM_A, 0).getBoost(FV_ATTACK), 0);
}


TEST_F(BasicEngineTest, SpeedTie) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));

  result.printStates();
  EXPECT_EQ(result.size(), 18);
  EXPECT_EQ(result.getNumUnique(), 9);
  EXPECT_EQ(result.whereHit(0).size(), 6);
  EXPECT_EQ(result.whereMiss(0).size(), 3);
  EXPECT_EQ(result.whereCrit(0).size(), 3);
}


TEST_F(BasicEngineTest, ReturnAllStates_Random) {
  engine_->setStateSelectMethod(NeoPkCU::StateSelectMethod::RANDOM);
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  EXPECT_EQ(result.size(), 1);
}


TEST_F(BasicEngineTest, ReturnAllStates_MostLikely) {
  engine_->setStateSelectMethod(NeoPkCU::StateSelectMethod::MOST_LIKELY);
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  EXPECT_EQ(result.size(), 1);
}


TEST_F(BasicEngineTest, HighEngineAccuracy_WithSpeedTie) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);

  // Two targeted moves with secondary effect
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::move(1));

  result.printStates();

  EXPECT_EQ(result.size(), 8450);
  EXPECT_EQ(result.getNumUnique(), 2401);
}


TEST_F(BasicEngineTest, HighEngineAccuracy_SingleMove) {
  spdlog::set_level(spdlog::level::warn);
  engine_->setAccuracy(16);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  result.printStates();

  EXPECT_EQ(result.size(), 65);
  EXPECT_EQ(result.getNumUnique(), 49);
}


TEST_F(BasicEngineTest, StateTransitionPrinterMiss) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  auto miss_state = result.where1Miss(0);
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), miss_state, false);
  EXPECT_TRUE(
      output.find("test_pokemon's attack missed!") != std::string::npos);
}


TEST_F(BasicEngineTest, StateTransitionPrinterDamage) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::wait(), Action::move(0));

  auto state = result.where1Hit(TEAM_B);
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), state, false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(
      output.find("test_pokemon lost 48 HP (15.5%)!") != std::string::npos);
}


TEST_F(BasicEngineTest, StateTransitionPrinterCritAndStatus) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  auto state = result.where1([](const ConstEnvironmentPossible& res) {
    return res.flagsFor(TEAM_A, 0).isCrit() &&
           res.teammate(TEAM_B, 0).getStatusAilment() == AIL_NV_POISON;
  });

  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), state, false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(
      output.find("test_pokemon scored a critical hit!") != std::string::npos);
  EXPECT_TRUE(
      output.find("test_pokemon lost 97 HP (31.3%)!") != std::string::npos);
  EXPECT_TRUE(output.find("test_pokemon was poisoned!") != std::string::npos);
}


TEST_F(BasicEngineTest, StateTransitionPrinterFaintAndSwitch) {
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::wait(), Action::swap(1));

  auto turn2 =
      engine_->updateState(turn1.where1(), Action::wait(), Action::move(1));

  ActionMap actionsA = {{{TEAM_A, 1}, Action::activate()}};
  ActionMap actionsB = {{{TEAM_B, 1}, Action::wait()}};
  auto turn3 = engine_->updateState(turn2.where1(), actionsA, actionsB);

  auto output = StateTransitionPrinter::printString(
      turn1.where1().getEnv(), turn3.where1(), false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(
      output.find("test_pokemon lost 310 HP (100.0%)!") != std::string::npos);
  EXPECT_TRUE(output.find("test_pokemon fainted!") != std::string::npos);
  EXPECT_TRUE(
      output.find("Team A sent out test_pokemon2!") != std::string::npos);
}


TEST_F(BasicEngineTest, StateTransitionPrinterDamageOnSwitch) {
  auto switch_and_attack = engine_->updateState(
      engine_->initialState(), Action::swap(1), Action::move(0));

  auto state = switch_and_attack.where1Hit(TEAM_B);
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), state, false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(
      output.find("test_pokemon2 lost 48 HP (15.5%)!") != std::string::npos);
}