#include "engine_test.hpp"
#include "pokemonai/state_transition_printer.h"


class SandstormTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey"))
          .addMove(pokedex_->move("sandstorm"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("tyranitar"))
          .setLevel(50));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("gengar"))
          .addMove(pokedex_->move("shadow ball"))
          .setLevel(50));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupSandstormTurn1() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupSandstormTurn2() {
    auto turn1 = setupSandstormTurn1();
    return engine_->updateState(
        turn1.where1(), Action::wait(), Action::wait());
  }

  PossibleEnvironments setupSandstormTurn3() {
    auto turn2 = setupSandstormTurn2();
    return engine_->updateState(
        turn2.where1(), Action::wait(), Action::wait());
  }

  PossibleEnvironments setupSandstormTurn4() {
    auto turn3 = setupSandstormTurn3();
    return engine_->updateState(
        turn3.where1(), Action::wait(), Action::wait());
  }

  PossibleEnvironments setupSandstormTurn5() {
    auto turn4 = setupSandstormTurn4();
    return engine_->updateState(
        turn4.where1(), Action::wait(), Action::wait());
  }

  PossibleEnvironments setupShadowBallNormal() {
    return engine_->updateState(
        engine_->initialState(), Action::swap(1), Action::move(0));
  }

  PossibleEnvironments setupShadowBallSandstorm() {
    auto turn1 = setupSandstormTurn1();
    return engine_->updateState(
        turn1.where1(), Action::swap(1), Action::move(0));
  }
};


TEST_F(SandstormTest, InitialWeatherIsNormal) {
  auto state_init = engine_->initialState();
  EXPECT_EQ(state_init.getTeam(TEAM_A).status().weather_type, WEATHER_NORMAL);
}


TEST_F(SandstormTest, SandstormSetsWeatherOnTurn1) {
  auto state_init = engine_->initialState();
  auto turn1 = setupSandstormTurn1();
  auto state1 = turn1.where1();

  EXPECT_EQ(state1.getTeam(TEAM_A).status().weather_type, WEATHER_SAND);
  EXPECT_EQ(state1.getTeam(TEAM_A).status().weather_duration, 4U);

  auto output1 = StateTransitionPrinter::printString(state_init, state1, false);
  SCOPED_TRACE(output1);
  EXPECT_TRUE(output1.find("A sandstorm kicked up!") != std::string::npos);
}


TEST_F(SandstormTest, SandstormTicksOnTurn2) {
  auto turn2 = setupSandstormTurn2();
  auto state2 = turn2.where1();
  EXPECT_EQ(state2.getTeam(TEAM_A).status().weather_type, WEATHER_SAND);
  EXPECT_EQ(state2.getTeam(TEAM_A).status().weather_duration, 3U);
}


TEST_F(SandstormTest, SandstormTicksOnTurn3) {
  auto turn3 = setupSandstormTurn3();
  auto state3 = turn3.where1();
  EXPECT_EQ(state3.getTeam(TEAM_A).status().weather_type, WEATHER_SAND);
  EXPECT_EQ(state3.getTeam(TEAM_A).status().weather_duration, 2U);
}


TEST_F(SandstormTest, SandstormTicksOnTurn4) {
  auto turn4 = setupSandstormTurn4();
  auto state4 = turn4.where1();
  EXPECT_EQ(state4.getTeam(TEAM_A).status().weather_type, WEATHER_SAND);
  EXPECT_EQ(state4.getTeam(TEAM_A).status().weather_duration, 1U);
}


TEST_F(SandstormTest, SandstormSubsidesOnTurn5) {
  auto turn5 = setupSandstormTurn5();
  auto state5 = turn5.where1();
  EXPECT_EQ(state5.getTeam(TEAM_A).status().weather_type, WEATHER_NORMAL);
  EXPECT_EQ(state5.getTeam(TEAM_A).status().weather_duration, 0U);

  auto turn4_prev = setupSandstormTurn4();
  auto output5 = StateTransitionPrinter::printString(turn4_prev.where1().getEnv(), state5, false);
  SCOPED_TRACE(output5);
  EXPECT_TRUE(output5.find("The sandstorm subsided.") != std::string::npos);
}


class SandstormDamageAndImmunityTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("blissey")) // Normal type (takes damage)
          .addMove(pokedex_->move("sandstorm"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("tyranitar")) // Rock type (immune)
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("clefable")) // Normal type, Magic Guard (immune)
          .setAbility(pokedex_->ability("magic guard"))
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("cacturne")) // Grass/Dark, Sand Veil (immune)
          .setAbility(pokedex_->ability("sand veil"))
          .setLevel(50));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("steelix")) // Steel type (immune)
          .setLevel(50))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("garchomp")) // Ground/Dragon type (immune)
          .setLevel(50));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);
  }

  PossibleEnvironments setupSandstormActive() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupSwapsTurn2() {
    auto turn1 = setupSandstormActive();
    return engine_->updateState(
        turn1.where1(), Action::swap(2), Action::swap(1));
  }

  PossibleEnvironments setupSwapsTurn3() {
    auto turn2 = setupSwapsTurn2();
    return engine_->updateState(
        turn2.where1(), Action::swap(3), Action::wait());
  }
};


TEST_F(SandstormDamageAndImmunityTest, BlisseyTakesSandstormDamageTurn1) {
  auto turn1 = setupSandstormActive();
  auto state1 = turn1.where1();
  auto blissey = state1.teammate(TEAM_A, 0);
  EXPECT_LT(blissey.getHP(), blissey.nv().getMaxHP());
}


TEST_F(SandstormDamageAndImmunityTest, SteelixAvoidsSandstormDamageTurn1) {
  auto turn1 = setupSandstormActive();
  auto state1 = turn1.where1();
  auto steelix = state1.teammate(TEAM_B, 0);
  EXPECT_EQ(steelix.getHP(), steelix.nv().getMaxHP());
}


TEST_F(
    SandstormDamageAndImmunityTest, SandstormDamageAvoidedByAbilitiesAndTypes) {
  auto turn2 = setupSwapsTurn2();
  auto state2 = turn2.where1();
  auto clefable = state2.teammate(TEAM_A, 2);
  auto garchomp = state2.teammate(TEAM_B, 1);

  EXPECT_EQ(clefable.getHP(), clefable.nv().getMaxHP());
  EXPECT_EQ(garchomp.getHP(), garchomp.nv().getMaxHP());
}


TEST_F(SandstormDamageAndImmunityTest, CacturneAvoidsSandstormDamageTurn3) {
  auto turn3 = setupSwapsTurn3();
  auto state3 = turn3.where1();
  auto cacturne = state3.teammate(TEAM_A, 3);
  EXPECT_EQ(cacturne.getHP(), cacturne.nv().getMaxHP());
}


TEST_F(SandstormTest, RockTypeSpecialDefenseBoost) {
  // Test damage of Special move (Shadow Ball) on Tyranitar (Rock type)
  // in normal weather vs sandstorm.
  
  // Normal weather:
  auto turn_normal = setupShadowBallNormal();
  auto state_normal = turn_normal.where1Status(TEAM_B); // use status roll where Gengar hit Tyranitar
  auto tyranitar_normal = state_normal.teammate(TEAM_A, 1);
  uint32_t damage_normal = tyranitar_normal.nv().getMaxHP() - tyranitar_normal.getHP();

  // Sandstorm weather:
  auto turn_sand = setupShadowBallSandstorm();
  auto state_sand = turn_sand.where1Status(TEAM_B);
  auto tyranitar_sand = state_sand.teammate(TEAM_A, 1);
  
  // Under Sandstorm, Tyranitar gets Sp. Def boost, but it also takes weather chip damage.
  // Tyranitar is Rock type and immune to sandstorm damage.
  // So the HP loss of Tyranitar is strictly from Shadow Ball.
  uint32_t damage_sand = tyranitar_sand.nv().getMaxHP() - tyranitar_sand.getHP();

  // Verify damage in Sandstorm is less than damage in normal weather
  EXPECT_LT(damage_sand, damage_normal);
  
  // Sp. Def is boosted by 1.5, meaning damage is scaled by (1.0 / 1.5) = 2/3.
  // Due to rounding, damage_sand should be approximately damage_normal * 2 / 3.
  // Allow a small range for rounding.
  uint32_t expected_sand_dmg = (damage_normal * 2) / 3;
  EXPECT_NEAR(damage_sand, expected_sand_dmg, 2.0);
}


TEST_F(SandstormTest, EnvironmentVolatilePrintsWeather) {
  auto turn1 = setupSandstormTurn1();
  auto state1 = turn1.where1();

  std::stringstream ss;
  ss << state1.getEnv();
  std::string output = ss.str();

  EXPECT_TRUE(
      output.find("Weather: Sandstorm (4 turns left)") != std::string::npos);
}
