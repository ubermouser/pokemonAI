#include "engine_test.hpp"


class NaturalCureTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("starmie"))
          .setAbility(pokedex_->ability("natural cure"))
          .addMove(pokedex_->move("surf"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("chansey"))
          .addMove(pokedex_->move("softboiled"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("jolteon"))
          .addMove(pokedex_->move("thunder wave"))
          .setLevel(100));
    auto environment = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment);
    auto env_v = engine_->initialState();

    turn1_twave = engine_->updateState(
      env_v, Action::wait(), Action::move(0));
    turn2_swap = engine_->updateState(
        turn1_twave.where1(), Action::swap(1), Action::wait());
  }

  PossibleEnvironments turn1_twave;
  PossibleEnvironments turn2_swap;
};


TEST_F(NaturalCureTest, NaturalCureDoesNotHealInBattle) {
  // Turn 1: Jolteon uses Thunder Wave on Starmie
  ASSERT_GE(turn1_twave.size(), 1);
  auto starmie_after_twave = turn1_twave.where1().teammate(TEAM_A, 0);
  EXPECT_EQ(starmie_after_twave.getStatusAilment(), AIL_NV_PARALYSIS);
}


TEST_F(NaturalCureTest, NaturalCureHealsOnSwitch) {
  // Turn 2: Starmie switches out
  ASSERT_GE(turn2_swap.size(), 1);
  auto starmie_after_switch = turn2_swap.where1().teammate(TEAM_A, 0);
  EXPECT_EQ(starmie_after_switch.getStatusAilment(), AIL_NV_NONE);
}


TEST_F(NaturalCureTest, NaturalCureReportedOnSwitch) {
  // Turn 2: Starmie switches out
  auto output = StateTransitionPrinter::printString(
      turn1_twave.where1().getEnv(), turn2_swap.where1(), false);

  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("starmie woke up / was cured") != std::string::npos);
}
