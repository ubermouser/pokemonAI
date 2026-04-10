#include <fmt/ranges.h>

#include "engine_test.hpp"


class Gen4BasicEngineTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("charmander"))
            .addMove(pokedex_->move("cut"))
            .addMove(pokedex_->move("fire blast"))
            .addMove(pokedex_->move("swords dance")))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("aipom"))
            .addMove(pokedex_->move("screech")));
    // clang-format on
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);
  }
};


TEST_F(Gen4BasicEngineTest, BuffStat) {
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
  EXPECT_EQ(result.where1().teammate(0, 0).getBoost(FV_ATTACK), 2);
}


TEST_F(Gen4BasicEngineTest, DebuffStat) {
  auto team_a = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("aipom"))
          .addMove(pokedex_->move("screech")));
  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle"))
          .addMove(pokedex_->move("tail whip")));
  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  result.printStates();
  // screech has 85 S.Accuracy, so it can hit or miss.
  // It has --- P.Accuracy.
  EXPECT_EQ(result.size(), 2);

  auto hitState = result.where1Hit(0);
  EXPECT_EQ(hitState.flagsFor(TEAM_A).isHit(), true);
  EXPECT_EQ(hitState.flagsFor(TEAM_A).isSecondary(), true);
  EXPECT_EQ(hitState.teammate(1, 0).getBoost(FV_DEFENSE), -2);

  auto missState = result.where1([](const ConstEnvironmentPossible& state) {
    return !state.flagsFor(TEAM_A).isSecondary();
  });
  EXPECT_EQ(missState.flagsFor(TEAM_A).isHit(), true);
  EXPECT_EQ(missState.flagsFor(TEAM_A).isSecondary(), false);
  EXPECT_EQ(missState.teammate(1, 0).getBoost(FV_DEFENSE), 0);
}


TEST_F(Gen4BasicEngineTest, StatusHitAndMiss) {
  auto team_a = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("arbok"))
          .addMove(pokedex_->move("glare")));
  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("cut")));
  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  result.printStates();
  // Glare has 75% accuracy and no damage/crit.
  // It should produce two states: hit and miss.
  EXPECT_EQ(result.size(), 2);

  auto hit_state = result.at(0);
  EXPECT_EQ(
      hit_state.getTeam(1).teammate(0).getStatusAilment(), AIL_NV_PARALYSIS);

  auto miss_state = result.at(1);
  EXPECT_EQ(miss_state.getTeam(1).teammate(0).getStatusAilment(), AIL_NV_NONE);
}


TEST_F(Gen4BasicEngineTest, HighEvasionAndAccuracy) {
  spdlog::set_level(spdlog::level::warn);
  // Reproduce branchProbability > FixType(0) assertion failure
  engine_->setAccuracy(16);
  auto team_a = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("mud-slap")));

  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("sweet scent")));

  auto environment_nv =
      std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
  engine_->setEnvironment(environment_nv);

  PossibleEnvironments results = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));
  auto currentState = results.where1Hit(TEAM_A);
  results.printStates();

  // Run 7 turns of accuracy/evasion debuffs
  for (int turn = 1; turn <= 7; ++turn) {
    std::cout << "Turn " << turn << ": Mud-slap vs Sweet Scent" << std::endl;
    // We want to ensure Mud-slap hits to keep the debuff loop going.
    // results.whereHit(TEAM_A) returns states where Team A's move hit.
    results =
        engine_->updateState(currentState, Action::move(0), Action::move(0));
    results.printStates();

    ASSERT_FALSE(results.empty())
        << "Engine returned no states on turn " << turn;

    // will throw if Mud-slap fails to hit
    currentState = results.where1Hit(TEAM_A);
  }

  std::cout << "Successfully completed 7 turns of Mud-slap vs Sweet Scent"
            << std::endl;
}
