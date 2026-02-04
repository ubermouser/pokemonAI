#include "engine_test.hpp"

class ToxicOrbTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .addMove(pokedex_->move("growl"))
          .setLevel(5));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magikarp"))
          .setInitialItem(pokedex_->item("toxic orb"))
          .addMove(pokedex_->move("splash"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    auto target_untouched = engine_->initialState();

    // Both use their moves (index 0)
    target_end_of_turn = engine_->updateState(
        target_untouched, Action::move(0), Action::move(0));
  }

  PossibleEnvironments target_end_of_turn;
};


TEST_F(ToxicOrbTest, ToxicOrbInducesBadPoison) {
  // Initially not poisoned
  auto target_untouched = engine_->initialState();
  ASSERT_EQ(target_untouched.getTeam(1).getPKV().getStatusAilment(), AIL_NV_NONE);

  // Check if alive
  ASSERT_TRUE(target_end_of_turn.where1().getTeam(1).getPKV().isAlive());

  // Magikarp should be badly poisoned at the end of the turn
  ASSERT_EQ(target_end_of_turn.where1().getTeam(1).getPKV().getStatusAilment(), AIL_NV_POISON_TOXIC);
}


TEST_F(ToxicOrbTest, ToxicOrbNotConsumed) {
  // Magikarp should still have the item
  ASSERT_TRUE(target_end_of_turn.where1().getTeam(1).getPKV().hasItem());
}

class ToxicOrbImmunityTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .addMove(pokedex_->move("growl"))
          .setLevel(5));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magnemite")) // Steel type
          .setInitialItem(pokedex_->item("toxic orb"))
          .addMove(pokedex_->move("double-edge")) // Valid move
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    auto target_untouched = engine_->initialState();

    // Both use their moves (index 0)
    target_end_of_turn = engine_->updateState(
        target_untouched, Action::move(0), Action::move(0));
  }

  PossibleEnvironments target_end_of_turn;
};

TEST_F(ToxicOrbImmunityTest, SteelTypeNotPoisoned) {
  // Magnemite should NOT be poisoned
  ASSERT_EQ(target_end_of_turn.where1().getTeam(1).getPKV().getStatusAilment(), AIL_NV_NONE);
}

class ToxicOrbOverwriteTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .addMove(pokedex_->move("growl"))
          .setLevel(5));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magikarp"))
          .setInitialItem(pokedex_->item("toxic orb"))
          .addMove(pokedex_->move("splash"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    auto c_env_v = engine_->initialState();
    EnvironmentVolatileData env_data = c_env_v.data();
    EnvironmentVolatile env_v(c_env_v.nv(), env_data);

    // Set initial status to Burn
    env_v.getTeam(1).getPKV().setStatusAilment(AIL_NV_BURN);

    // Both use their moves (index 0)
    target_end_of_turn =
        engine_->updateState(env_v, Action::move(0), Action::move(0));
  }

  PossibleEnvironments target_end_of_turn;
};

TEST_F(ToxicOrbOverwriteTest, OverwritesBurn) {
  // Magikarp should be badly poisoned at the end of the turn, overwriting Burn
  ASSERT_EQ(target_end_of_turn.where1().getTeam(1).getPKV().getStatusAilment(), AIL_NV_POISON_TOXIC);
}
