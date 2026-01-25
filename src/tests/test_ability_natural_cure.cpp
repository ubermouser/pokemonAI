#include <sstream>

#include "engine_test.hpp"
#include "pokemonai/state_transition_printer.h"


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
      turn1_twave.at(0), Action::swap(1), Action::wait());
  }

  PossibleEnvironments turn1_twave;
  PossibleEnvironments turn2_swap;
};


TEST_F(NaturalCureTest, NaturalCureDoesNotHealInBattle) {
  // Turn 1: Jolteon uses Thunder Wave on Starmie
  ASSERT_EQ(turn1_twave.size(), 1);
  auto starmie_after_twave = turn1_twave.at(0).getEnv().getTeam(TEAM_A).getPKV();
  EXPECT_EQ(starmie_after_twave.getStatusAilment(), AIL_NV_PARALYSIS);
}


TEST_F(NaturalCureTest, NaturalCureHealsOnSwitch) {
  // Turn 2: Starmie switches out
  ASSERT_EQ(turn2_swap.size(), 1);
  auto starmie_after_switch = turn2_swap.at(0).getEnv().getTeam(TEAM_A).teammate(0);
  EXPECT_EQ(starmie_after_switch.getStatusAilment(), AIL_NV_NONE);
}


TEST_F(NaturalCureTest, NaturalCureReportedOnSwitch) {
  // Turn 2: Starmie switches out
  const auto& newState = turn2_swap.at(0);
  const auto& oldState = turn1_twave.at(0);  // Before Turn 2

  std::stringstream ss;
  StateTransitionPrinter::print(ss, oldState.getEnv(), newState);

  std::string output = ss.str();
  SCOPED_TRACE(output);
  EXPECT_TRUE(output.find("starmie") != std::string::npos);
  EXPECT_TRUE(output.find("cured") != std::string::npos);
}
