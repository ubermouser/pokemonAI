#include "engine_test.hpp"


class UTurnTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("scizor"))
          .addMove(pokedex_->move("u-turn"))
          .setInitialItem(pokedex_->item("life orb"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("torterra"))
          .addMove(pokedex_->move("stealth rock"))
          .setLevel(50));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setup_TA0_vs_TB1() {
    return engine_->updateState(
        engine_->initialState(), Action::wait(), Action::swap(1));
  }

  PossibleEnvironments setupSR() {
    auto swap = setup_TA0_vs_TB1();
    return engine_->updateState(
        swap.where1(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments setup_TA1_vs_TB1_UTurn() {
    auto swap = setup_TA0_vs_TB1();
    return engine_->updateState(
        swap.where1(), Action::moveAlly(0, 1), Action::wait());
  }

  PossibleEnvironments setupUTurnToAllyWithSR() {
    auto sr = setupSR();
    return engine_->updateState(
        sr.where1(), Action::moveAlly(0, 1), Action::wait());
  }

  PossibleEnvironments setup_TA1_vs_TB0_TB1_fainted() {
    auto uturn = setup_TA1_vs_TB1_UTurn();
    // Team B's Torterra (index 1) has fainted. Scizor (index 0) must enter.
    ActionMap actionsA = {{{TEAM_A, 1}, Action::wait()}};
    ActionMap actionsB = {{{TEAM_B, 0}, Action::activate()}};
    return engine_->updateState(uturn.where1(), actionsA, actionsB);
  }

  PossibleEnvironments setupUTurnNoAlly() {
    auto scizor = setup_TA1_vs_TB0_TB1_fainted();
    return engine_->updateState(
        scizor.where1(), Action::wait(), Action::moveAlly(0, 0));
  }
};


TEST_F(UTurnTest, requires_pokemon_to_swap_if_ally_exists) {
  auto sr = setupSR();
  EXPECT_FALSE(
      engine_->isValidAction(sr.where1().getEnv(), Actor(TEAM_A, 0), Action::move(0)));
  EXPECT_TRUE(engine_->isValidAction(
      sr.where1().getEnv(), Actor(TEAM_A, 0), Action::moveAlly(0, 1)));
  EXPECT_FALSE(engine_->isValidAction(
      sr.where1().getEnv(), Actor(TEAM_A, 0), Action::struggle()));
}


TEST_F(UTurnTest, can_still_be_used_without_swap_if_no_allies_exist) {
  auto scizor = setup_TA1_vs_TB0_TB1_fainted();
  EXPECT_FALSE(
      engine_->isValidAction(scizor.where1().getEnv(), Actor(TEAM_B, 0), Action::move(0)));
  EXPECT_TRUE(engine_->isValidAction(
      scizor.where1().getEnv(), Actor(TEAM_B, 0), Action::moveAlly(0, 0)));
  EXPECT_FALSE(engine_->isValidAction(
      scizor.where1().getEnv(), Actor(TEAM_B, 0), Action::moveAlly(0, 1)));
}


TEST_F(UTurnTest, damages_enemy_and_swaps_to_ally) {
  auto turn = setup_TA1_vs_TB1_UTurn();
  // pp decremented
  EXPECT_EQ(turn.where1Hit(0).teammate(0, 0).getMV(0).getPP(), 31);
  // item effect (life orb) applies
  EXPECT_NEAR(
      turn.where1Hit(0).teammate(0, 0).getPercentHP(), 0.9, 0.005);
  // ally has swapped out
  EXPECT_FALSE(turn.where1Hit(0).teammate(0, 0).isActive());
  EXPECT_TRUE(turn.where1Hit(0).teammate(0, 1).isActive());
  EXPECT_EQ(
      turn.where1Hit(0).teammate(1, 1).getPercentHP(),
      0.);  // enemy weakling deleted
}


TEST_F(UTurnTest, damages_enemy_and_swaps_to_ally_with_stealth_rock) {
  auto turn = setupUTurnToAllyWithSR();
  // life orb applies to attacking teammate
  EXPECT_NEAR(
      turn.where1Hit(0).teammate(0, 0).getPercentHP(),
      0.9,
      0.005);
  // stealth-rock applies to entering teammate
  EXPECT_NEAR(
      turn.where1Hit(0).teammate(0, 1).getPercentHP(),
      0.9375,
      0.005);
}


TEST_F(UTurnTest, damages_enemy_but_doesnt_swap_if_no_allies_exist) {
  auto turn = setupUTurnNoAlly();
  // pp decremented
  EXPECT_EQ(turn.where1Hit(1).teammate(1, 0).getMV(0).getPP(), 31);
  // item effect (life orb) applies
  EXPECT_FLOAT_EQ(
      turn.where1Hit(1).teammate(1, 0).getPercentHP(), 0.9);
  // item effect (life orb) applies
  EXPECT_FLOAT_EQ(
      turn.where1Hit(1).teammate(1, 0).getPercentHP(), 0.9);
  // ally NOT swapped out
  EXPECT_TRUE(turn.where1Hit(1).teammate(1, 0).isActive());
}


TEST_F(UTurnTest, NoErroneousStruggleWithChoiceItem) {
  // Setup: Team with only Scizor to ensure U-turn doesn't swap out
  auto team_cb = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("scizor"))
        .addMove(pokedex_->move("u-turn"))
        .addMove(pokedex_->move("bullet punch"))
        .setInitialItem(pokedex_->item("choice band"))
        .setLevel(100));

  engine_->setEnvironment(EnvironmentNonvolatile(team_cb, team_cb, true));

  // Turn 1: Use U-turn. Since it's the last pokemon, it stays in.
  // We use moveAlly(0, 0) because U-turn requires a friendly target.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::moveAlly(0, 0), Action::wait());
  auto state = turn1.where1Hit(0);

  // Verify Choice Band lock: Bullet Punch (index 1) should be disabled
  EXPECT_FALSE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::move(1)));

  // Verify U-turn (index 0) remains enabled (for target 0)
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::moveAlly(0, 0)));

  // Struggle should be disabled because U-turn is available.
  EXPECT_FALSE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::struggle()));
}


TEST_F(UTurnTest, UTurnReported) {
  // Turn 1: Scizor uses U-turn on enemy Torterra and swaps to friendly Torterra
  auto swap = setup_TA0_vs_TB1();
  auto uturn = setup_TA1_vs_TB1_UTurn();
  auto output = StateTransitionPrinter::printString(
      swap.where1().getEnv(), uturn.where1Hit(0), false);

  SCOPED_TRACE(output);
  // Verify swap action is reported
  EXPECT_TRUE(output.find("Team A sent out torterra!") != std::string::npos);

  // Verify damage to target is reported
  EXPECT_TRUE(output.find("torterra lost") != std::string::npos);
  EXPECT_TRUE(output.find("HP") != std::string::npos);
}


TEST_F(UTurnTest, SwappedInPokemonShouldNotBeEncored) {
  // Re-setup with Team B having a slower Encore user (Shuckle)
  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("shuckle"))
          .addMove(pokedex_->move("encore"))
          .setLevel(100));

  engine_->setEnvironment(
      EnvironmentNonvolatile(environment_nv.getTeam(0), team_b, true));

  // Turn 1: Scizor uses U-turn (moveAlly 0, 1), Shuckle uses Encore (move 0)
  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::moveAlly(0, 1), Action::move(0));

  auto state = turn1.where1Hit(0);

  // Torterra (Team A, index 1) is now active.
  EXPECT_TRUE(state.teammate(0, 1).isActive());

  // Verify Torterra is NOT encored.
  EXPECT_EQ(state.teammate(0, 1).status().encore_duration, 0)
      << "Torterra should not be encored as it has not used a move yet.";
}
