#include <gtest/gtest.h>

#include <memory>

#include "pokemonai/engine.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pkCU.h"

class AbilitiesTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;

  uint32_t getDamage(const std::string& pokemon, const std::string& ability, const std::string& move, bool lowHP) {
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon(pokemon))
          .setAbility(pokedex_->ability(ability))
          .addMove(pokedex_->move(move))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("snorlax"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    auto environment = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment);
    auto initial_state = engine_->initialState();

    // Create a mutable copy of the environment data
    EnvironmentVolatileData mutable_data = initial_state.data();
    EnvironmentVolatile env_v(initial_state.nv(), mutable_data);

    if (lowHP) {
        env_v.getTeam(TEAM_A).cSetPercentHP(0.3);
    }

    auto turn_outcome = engine_->updateState(
      env_v, Action::move(0), Action::wait());

    if (turn_outcome.empty()) return 0;

    // Use the first outcome (most probable or default)
    auto target_pkv = turn_outcome.at(0).getEnv().getTeam(TEAM_B).getPKV();
    return target_pkv.getMissingHP();
  }
};


TEST_F(AbilitiesTest, Blaze) {
  // Charizard with Blaze using Flamethrower
  uint32_t normalDamage = getDamage("charizard", "blaze", "flamethrower", false);
  uint32_t boostedDamage = getDamage("charizard", "blaze", "flamethrower", true);

  // Expect boosted damage to be roughly 1.5x normal damage
  // Due to random damage rolls (0.85-1.0), ranges are:
  // Normal: 0.85*D - 1.0*D
  // Boosted: 1.5 * (0.85*D - 1.0*D) = 1.275*D - 1.5*D
  // Boosted min (1.275) > Normal max (1.0).
  EXPECT_GT(boostedDamage, normalDamage);
}

TEST_F(AbilitiesTest, Overgrow) {
  // Venusaur with Overgrow using Energy Ball (Vine Whip is missing in gen4_moves.txt)
  uint32_t normalDamage = getDamage("venusaur", "overgrow", "energy ball", false);
  uint32_t boostedDamage = getDamage("venusaur", "overgrow", "energy ball", true);
  EXPECT_GT(boostedDamage, normalDamage);
}

TEST_F(AbilitiesTest, Swarm) {
  // Scyther with Swarm using X-Scissor
  uint32_t normalDamage = getDamage("scyther", "swarm", "x-scissor", false);
  uint32_t boostedDamage = getDamage("scyther", "swarm", "x-scissor", true);
  EXPECT_GT(boostedDamage, normalDamage);
}

TEST_F(AbilitiesTest, Torrent) {
  // Blastoise with Torrent using Water Gun
  uint32_t normalDamage = getDamage("blastoise", "torrent", "water gun", false);
  uint32_t boostedDamage = getDamage("blastoise", "torrent", "water gun", true);
  EXPECT_GT(boostedDamage, normalDamage);
}

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
