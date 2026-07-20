#include "engine_test.hpp"


class SandStreamTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    // Team A: Charmander (Lead), Tyranitar (Sand Stream)
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("growl"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("tyranitar"))
          .setAbility(pokedex_->ability("sand stream"))
          .addMove(pokedex_->move("earthquake"))
          .setLevel(100));

    // Team B: Charmander (Lead)
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("growl"))
          .setLevel(100));
    // clang-format on

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

    setup_normal = engine_->updateState(
        engine_->initialState(), Action::wait(), Action::wait());
  }

  PossibleEnvironments setup_normal;

  PossibleEnvironments setupSandStreamTurn1() {
    return engine_->updateState(
        setup_normal.where1(), Action::swap(1), Action::wait());
  }
};


TEST_F(SandStreamTest, SandStreamSetsWeatherOnSwitchIn) {
  auto state_init = setup_normal.where1();
  auto turn1 = setupSandStreamTurn1();
  auto state1 = turn1.where1();

  EXPECT_EQ(state1.getTeam(TEAM_A).status().weather_type, WEATHER_SAND);
  EXPECT_EQ(state1.getTeam(TEAM_A).status().weather_duration, 4U);

  auto output1 = StateTransitionPrinter::printString(state_init, state1, false);
  SCOPED_TRACE(output1);
  EXPECT_TRUE(output1.find("A sandstorm kicked up!") != std::string::npos);
}


TEST_F(SandStreamTest, SandStreamTriggersAtStartOfBattle) {
  // clang-format off
  auto team_a_lead = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("tyranitar"))
        .setAbility(pokedex_->ability("sand stream"))
        .addMove(pokedex_->move("earthquake"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("charmander"))
        .addMove(pokedex_->move("growl"))
        .setLevel(100));

  auto team_b_lead = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("charmander"))
        .addMove(pokedex_->move("growl"))
        .setLevel(100));
  // clang-format on

  engine_->setEnvironment(
      EnvironmentNonvolatile(team_a_lead, team_b_lead, true));

  auto init_state = engine_->initialState();
  EXPECT_EQ(init_state.getTeam(TEAM_A).status().weather_type, WEATHER_SAND);
}
