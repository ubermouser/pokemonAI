#include "engine_test.hpp"

class SereneGraceTest : public Gen4EngineTest {
 protected:
  void SetUp() override { Gen4EngineTest::SetUp(); }
};

TEST_F(SereneGraceTest, SereneGraceDoublesSecondaryEffectChance) {
  // Jirachi (Serene Grace) extracts 2x probability from secondary effects
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("jirachi"))
        .setAbility(pokedex_->ability("serene grace"))
        .addMove(pokedex_->move("psychic")) // 10% chance to lower SpDef
        .setLevel(100));
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("starmie"))
        .setAbility(pokedex_->ability("natural cure"))
        .addMove(pokedex_->move("recover"))
        .setLevel(100));

  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);
  auto env_v = engine_->initialState();

  // Attack with Psychic
  auto turn1_outcome = engine_->updateState(
    env_v, Action::move(0), Action::wait());

  double secondary_prob_sum = 0.0;

  for (size_t i = 0; i < turn1_outcome.size(); ++i) {
    auto env = turn1_outcome.at(i);
    // TEAM_A (0) used the move.
    if (env.flagsFor(TEAM_A).isSecondary()) {
        secondary_prob_sum += env.getProbability().to_double();
    }
  }

  // Total probability should be close to 1.0 (assuming no misses).
  // Psychic 100% accuracy.
  
  // With Serene Grace: 20% chance.
  // Expected: secondary_prob_sum ~= 0.2.
  EXPECT_NEAR(secondary_prob_sum, 0.2, 0.001);
}

TEST_F(SereneGraceTest, ControlGroupCheck) {
  // Mewtwo (Pressure) standard 10% chance
  auto team_a = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("mewtwo"))
        .setAbility(pokedex_->ability("pressure"))
        .addMove(pokedex_->move("psychic")) 
        .setLevel(100));
  auto team_b = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("jirachi"))
        .addMove(pokedex_->move("psychic"))
        .setLevel(100));

  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);
  auto env_v = engine_->initialState();

  auto turn1_outcome = engine_->updateState(
    env_v, Action::move(0), Action::wait());

  double secondary_prob_sum = 0.0;
  for (size_t i = 0; i < turn1_outcome.size(); ++i) {
    auto env = turn1_outcome.at(i);
    if (env.flagsFor(TEAM_A).isSecondary()) {
        secondary_prob_sum += env.getProbability().to_double();
    }
  }

  // Without Serene Grace: 10% chance.
  EXPECT_NEAR(secondary_prob_sum, 0.1, 0.001);
}
