#include "engine_test.hpp"


class ScreenTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("reflect"))
          .addMove(pokedex_->move("light screen"))
          .addMove(pokedex_->move("strength")) // Physical
          .addMove(pokedex_->move("swift")) // Special
          .setIV(FV_DEFENSE, 30) // make sure pkmn survives
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mewtwo"))
          .addMove(pokedex_->move("strength")) // Physical
          .addMove(pokedex_->move("swift")) // Special
          .setIV(FV_DEFENSE, 30) // make sure pkmn survives
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    mew_reflect = engine_->updateState(engine_->initialState(), Action::moveSideAlly(0), Action::wait());
    mew_reflect_physdmg = engine_->updateState(
        mew_reflect.where1(), Action::wait(), Action::move(0));
    mew_lightscreen = engine_->updateState(engine_->initialState(), Action::moveSideAlly(1), Action::wait());
    mew_lightscreen_specdmg = engine_->updateState(
        mew_lightscreen.where1(), Action::wait(), Action::move(1));
    mew_physdmg = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(0));
    mew_specdmg = engine_->updateState(engine_->initialState(), Action::wait(), Action::move(1));
  }

  PossibleEnvironments mew_reflect;
  PossibleEnvironments mew_lightscreen;
  PossibleEnvironments mew_reflect_physdmg;
  PossibleEnvironments mew_lightscreen_specdmg;
  PossibleEnvironments mew_physdmg;
  PossibleEnvironments mew_specdmg;
};


TEST_F(ScreenTest, ReflectReducesPhysicalDamage) {
  auto damage_baseline = mew_physdmg.where1().teammate(0, 0).getMissingHP();
  auto damage_reflect =
      mew_reflect_physdmg.where1().teammate(0, 0).getMissingHP();

  EXPECT_EQ(mew_reflect.where1().getTeam(0).status().reflect, 5);
  EXPECT_LE(damage_reflect, damage_baseline * 0.5);
}


TEST_F(ScreenTest, LightScreenReducesSpecialDamage) {
  auto damage_baseline = mew_specdmg.where1().teammate(0, 0).getMissingHP();
  auto damage_screen =
      mew_lightscreen_specdmg.where1().teammate(0, 0).getMissingHP();

  EXPECT_EQ(
      mew_lightscreen.where1().getTeam(0).status().lightScreen, 5);
  EXPECT_LE(damage_screen, damage_baseline * 0.5);
}


TEST_F(ScreenTest, ReflectDurationDecrements) {
  // Turn 1: Mew uses Reflect
  auto result = mew_reflect;
  auto current_state = result.where1();
  EXPECT_EQ(current_state.getTeam(0).status().reflect, 5);

  // Turns 2-5: Reflect is active and decrements
  for (int i = 0; i < 4; ++i) {
      // Advance turn using a move instead of wait, to ensure beginning of turn hooks run
      result = engine_->updateState(current_state, Action::move(2), Action::move(0));
      current_state = result.where1();
      int expected_duration = 4 - i;
      EXPECT_EQ(
          current_state.getTeam(0).status().reflect, expected_duration)
          << "Turn " << (i + 2);
  }

  EXPECT_EQ(current_state.getTeam(0).status().reflect, 1);

  // Turn 6: Decrement to 0.
  auto turn6 = engine_->updateState(current_state, Action::move(2), Action::move(0));
  EXPECT_EQ(turn6.where1().getTeam(0).status().reflect, 0);
}


TEST_F(ScreenTest, LightScreenDurationDecrements) {
  // Turn 1: Mew uses Light Screen
  auto result = mew_lightscreen;
  auto current_state = result.where1();
  EXPECT_EQ(current_state.getTeam(0).status().lightScreen, 5);

  for (int i = 0; i < 4; ++i) {
      result = engine_->updateState(current_state, Action::move(2), Action::move(0));
      current_state = result.where1();
  }

  EXPECT_EQ(current_state.getTeam(0).status().lightScreen, 1);

  auto turn6 = engine_->updateState(current_state, Action::move(2), Action::move(0));
  current_state = result.where1();
  EXPECT_EQ(turn6.where1().getTeam(0).status().lightScreen, 0);
}


TEST_F(ScreenTest, FailsIfAlreadyActive) {
  // Turn 1: Mew uses Reflect
  auto turn1 = mew_reflect;
  EXPECT_EQ(turn1.where1().getTeam(0).status().reflect, 5);

  // Turn 2: Mew tries to use Reflect again.
  // Should decrement to 4, then fail to reset to 5.

  auto turn2 =
      engine_->updateState(turn1.where1(), Action::moveSideAlly(0), Action::wait());
  EXPECT_EQ(turn2.where1().getTeam(0).status().reflect, 4);
}


// TODO(@drendleman) - the lightscreen counter should decrement when the user
// is swapping, but not when the swap is free (friendly is dead)!