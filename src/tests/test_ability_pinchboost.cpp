#include "engine_test.hpp"


class PinchBoostTest : public EngineTest {
protected:

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


TEST_F(PinchBoostTest, Blaze) {
  // Charizard with Blaze using Flamethrower
  uint32_t normalDamage = getDamage("charizard", "blaze", "flamethrower", false);
  uint32_t boostedDamage = getDamage("charizard", "blaze", "flamethrower", true);

  // TODO(@drendleman) burn condition is messing with state selection
  EXPECT_GT(boostedDamage, normalDamage * 1.2);
}

TEST_F(PinchBoostTest, Overgrow) {
  // Venusaur with Overgrow using Energy Ball (Vine Whip is missing in gen4_moves.txt)
  uint32_t normalDamage = getDamage("venusaur", "overgrow", "energy ball", false);
  uint32_t boostedDamage = getDamage("venusaur", "overgrow", "energy ball", true);
  EXPECT_GT(boostedDamage, normalDamage * 1.4);
}

TEST_F(PinchBoostTest, Swarm) {
  // Scyther with Swarm using X-Scissor
  uint32_t normalDamage = getDamage("scyther", "swarm", "x-scissor", false);
  uint32_t boostedDamage = getDamage("scyther", "swarm", "x-scissor", true);
  EXPECT_GT(boostedDamage, normalDamage * 1.4);
}

TEST_F(PinchBoostTest, Torrent) {
  // Blastoise with Torrent using Water Gun
  uint32_t normalDamage = getDamage("blastoise", "torrent", "water gun", false);
  uint32_t boostedDamage = getDamage("blastoise", "torrent", "water gun", true);
  EXPECT_GT(boostedDamage, normalDamage * 1.4);
}
