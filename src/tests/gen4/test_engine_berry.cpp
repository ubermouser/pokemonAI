#include "engine_test.hpp"


class BerryTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("lapras"))
          .addMove(pokedex_->move("ice beam"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("garchomp"))
          .setInitialItem(pokedex_->item("yache berry"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gabite"))
          .setIV(FV_SPDEFENSE, 30)  // ensure SPD matches garchomp's
          .setLevel(100));
    environment_nv = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment_nv);

    lapras_v_berry = engine_->updateState(
      engine_->initialState(), Action::swap(1), Action::wait());
    lapras_v_no_berry = engine_->updateState(
      engine_->initialState(), Action::swap(2), Action::wait());
    result_berry = engine_->updateState(
      lapras_v_berry.where1(), Action::wait(), Action::move(0));
    result_control = engine_->updateState(
      lapras_v_no_berry.where1(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments lapras_v_berry;
  PossibleEnvironments lapras_v_no_berry;
  PossibleEnvironments result_berry;
  PossibleEnvironments result_control;
};


TEST_F(BerryTest, TypeResistingBerry_Consumption) {
  EXPECT_TRUE(lapras_v_berry.where1().teammate(TEAM_A, 1).hasItem());
  EXPECT_FALSE(result_berry.where1().teammate(TEAM_A, 1).hasItem());
}


TEST_F(BerryTest, TypeResistingBerry_DamageReduction) {
  uint32_t dmg_berry = result_berry.where1().teammate(0, 1).getMissingHP();
  uint32_t dmg_control = result_control.where1().teammate(0, 2).getMissingHP();
  
  ASSERT_GT(dmg_control, 0);
  EXPECT_NEAR((float)dmg_berry / dmg_control, 0.5f, 0.1f);
}


TEST_F(BerryTest, TypeResistingBerry_Message) {
  std::string output = StateTransitionPrinter::printString(
    lapras_v_berry.where1().getEnv(), result_berry.where1(), false);

  result_berry.printStates();

  SCOPED_TRACE(output);
  EXPECT_TRUE(
    output.find("garchomp used its yache berry") != std::string::npos);
}
