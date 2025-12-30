#include "engine_test.hpp"

class LevitateTest : public EngineTest {
protected:
  void SetUp() override {
    EngineTest::SetUp();
  }
};

TEST_F(LevitateTest, ImmunityToGroundMoves) {
  // Team A: Sandshrew with Earthquake
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("sandshrew"))
        .addMove(pokedex_->move("earthquake"))
        .setLevel(50));

  // Team B: Gastly (Natural Levitate)
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("gastly"))
        .setAbility(pokedex_->ability("levitate"))
        .addMove(pokedex_->move("shadow ball"))
        .setLevel(50));

  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

  // Sandshrew uses Earthquake on Gastly
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  auto gastly = turn1.at(0).getEnv().getTeam(TEAM_B).getPKV();

  // Should take no damage (HP equal to Max HP)
  EXPECT_EQ(gastly.getHP(), gastly.nv().getMaxHP());
}

TEST_F(LevitateTest, VulnerabilityWithoutLevitate) {
  // Team A: Sandshrew with Earthquake
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("sandshrew"))
        .addMove(pokedex_->move("earthquake"))
        .setLevel(50));

  // Team B: Grimer (Poison, weak to Ground)
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("grimer"))
        .setAbility(pokedex_->ability("sticky hold")) // Just giving it an ability
        .addMove(pokedex_->move("poison jab"))
        .setLevel(50));

  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

  // Sandshrew uses Earthquake on Grimer
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  auto grimer = turn1.at(0).getEnv().getTeam(TEAM_B).getPKV();

  // Should take damage
  EXPECT_LT(grimer.getHP(), grimer.nv().getMaxHP());
}

TEST_F(LevitateTest, ImmunityToSpikes) {
  // Team A: Forretress (Spikes)
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("forretress"))
        .addMove(pokedex_->move("spikes"))
        .setLevel(50));

  // Team B: Starter (Grimer), Switch-in (Gastly - Levitate)
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("grimer"))
        .addMove(pokedex_->move("poison jab"))
        .setLevel(50))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("gastly"))
        .setAbility(pokedex_->ability("levitate"))
        .addMove(pokedex_->move("shadow ball"))
        .setLevel(50));

  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

  // Turn 1: Forretress uses Spikes. Grimer waits.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  // Turn 2: Team B switches Grimer to Gastly. Team A waits.
  auto turn2 = engine_->updateState(turn1.at(0), Action::wait(), Action::swap(1));

  auto gastly = turn2.at(0).getEnv().getTeam(TEAM_B).getPKV();

  // Gastly should take no damage from Spikes
  EXPECT_EQ(gastly.getHP(), gastly.nv().getMaxHP());
}

TEST_F(LevitateTest, VulnerabilityToStealthRock) {
  // Team A: Forretress (Stealth Rock)
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("forretress"))
        .addMove(pokedex_->move("stealth rock"))
        .setLevel(50));

  // Team B: Starter (Grimer), Switch-in (Gastly - Levitate)
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("grimer"))
        .addMove(pokedex_->move("poison jab"))
        .setLevel(50))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("gastly"))
        .setAbility(pokedex_->ability("levitate"))
        .addMove(pokedex_->move("shadow ball"))
        .setLevel(50));

  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

  // Turn 1: Forretress uses Stealth Rock. Grimer waits.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  // Turn 2: Team B switches Grimer to Gastly. Team A waits.
  auto turn2 = engine_->updateState(turn1.at(0), Action::wait(), Action::swap(1));

  auto gastly = turn2.at(0).getEnv().getTeam(TEAM_B).getPKV();

  // Gastly should take damage from Stealth Rock (Levitate doesn't protect)
  EXPECT_LT(gastly.getHP(), gastly.nv().getMaxHP());
}

TEST_F(LevitateTest, ImmunityToToxicSpikes) {
  // Team A: Forretress (Toxic Spikes)
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("forretress"))
        .addMove(pokedex_->move("toxic spikes"))
        .setLevel(50));

  // Team B: Starter (Grimer), Switch-in (Flygon - Levitate, Ground/Dragon)
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("grimer"))
        .addMove(pokedex_->move("poison jab"))
        .setLevel(50))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("flygon"))
        .setAbility(pokedex_->ability("levitate"))
        .addMove(pokedex_->move("dragon claw"))
        .setLevel(50));

  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

  // Turn 1: Forretress uses Toxic Spikes. Grimer waits.
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::wait());

  // Turn 2: Team B switches Grimer to Flygon. Team A waits.
  auto turn2 = engine_->updateState(turn1.at(0), Action::wait(), Action::swap(1));

  auto flygon = turn2.at(0).getEnv().getTeam(TEAM_B).getPKV();

  // Flygon should not be poisoned
  EXPECT_EQ(flygon.getStatusAilment(), AIL_NV_NONE);
}
