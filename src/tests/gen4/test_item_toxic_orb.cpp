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

    auto target_untouched = engine_->initialState();

    // Set initial status to Burn
    // In order to modify the state, we need to const_cast or use a method that allows modification if available in test context.
    // Or simpler: simulate a turn where the pokemon gets burned first.
    // However, since we can't easily simulate burn without setting up a specific scenario,
    // we will rely on the fact that we can modify the volatile data if we have a mutable reference.
    // engine_->initialState() returns a ConstEnvironmentVolatile which doesn't allow modification.
    // We need to create a mutable EnvironmentVolatile if possible or hack it for the test.
    // But engine_->updateState takes a const ref.

    // Let's manually construct a state with burn? Or use a cheat?
    // Since this is a unit test, we can const_cast for setup if necessary, but better to use available APIs.
    // PokemonVolatile has setStatusAilment.
    // We need a mutable EnvironmentVolatile.

    // Let's create a new EnvironmentVolatileData from the initial state, modify it, and pass it to updateState.
    // But updateState takes ConstEnvironmentVolatile.
    // We can construct a ConstEnvironmentVolatile from EnvironmentVolatileData.

    // Actually, let's just cheat for the test setup since we don't have easy access to a "Make Burned" move in this minimal setup.
    const_cast<PokemonVolatileData&>(target_untouched.getTeam(1).getPKV().data()).status_nonvolatile = AIL_NV_BURN;

    // Both use their moves (index 0)
    target_end_of_turn = engine_->updateState(
        target_untouched, Action::move(0), Action::move(0));
  }

  PossibleEnvironments target_end_of_turn;
};

TEST_F(ToxicOrbOverwriteTest, OverwritesBurn) {
  // Magikarp should be badly poisoned at the end of the turn, overwriting Burn
  ASSERT_EQ(target_end_of_turn.where1().getTeam(1).getPKV().getStatusAilment(), AIL_NV_POISON_TOXIC);
}
