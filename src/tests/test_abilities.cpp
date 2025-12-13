#include "engine_test.hpp"

class AbilitiesTest : public EngineTest { };


TEST_F(AbilitiesTest, NaturalCure) {
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

  auto turn1_twave = engine_->updateState(
    env_v, Action::wait(), Action::move(0));
  auto turn2_swap = engine_->updateState(
    turn1_twave.at(0), Action::swap(1), Action::wait());

  { // Turn 1: Jolteon uses Thunder Wave on Starmie
    ASSERT_EQ(turn1_twave.size(), 1);
    auto starmie_after_twave = turn1_twave.at(0).getEnv().getTeam(TEAM_A).getPKV();
    EXPECT_EQ(starmie_after_twave.getStatusAilment(), AIL_NV_PARALYSIS);
  }
  { // Turn 2: Starmie switches out
    ASSERT_EQ(turn2_swap.size(), 1);
    auto starmie_after_switch = turn2_swap.at(0).getEnv().getTeam(TEAM_A).teammate(0);
    EXPECT_EQ(starmie_after_switch.getStatusAilment(), AIL_NV_NONE);
  }
}


TEST_F(AbilitiesTest, Pressure) {
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("starmie"))
        .addMove(pokedex_->move("water gun"))
        .setLevel(100));
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("mewtwo"))
        .setAbility(pokedex_->ability("pressure"))
        .addMove(pokedex_->move("psychic"))
        .setLevel(100));
  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);
  auto env_v = engine_->initialState();

  auto turn1_water_gun = engine_->updateState(
    env_v, Action::move(0), Action::wait());

  { // Turn 1: Starmie uses water gun
    EXPECT_EQ(turn1_water_gun.size(), 2);
    auto starmie_after_water_gun = turn1_water_gun.at(0).getEnv().getTeam(TEAM_A).getPKV();
    EXPECT_EQ(starmie_after_water_gun.getMV(0).getPP(), 38);
  }
}
