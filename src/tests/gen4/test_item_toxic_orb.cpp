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
        target_untouched, Action::move(0), Action::moveAlly(0, 0));
  }

  PossibleEnvironments target_end_of_turn;
};


TEST_F(ToxicOrbTest, ToxicOrbInducesBadPoison) {
  // Initially not poisoned
  auto target_untouched = engine_->initialState();
  ASSERT_EQ(target_untouched.teammate(1, 0).getStatusAilment(), AIL_NV_NONE);

  // Check if alive
  ASSERT_TRUE(target_end_of_turn.where1().teammate(1, 0).isAlive());

  // Magikarp should be badly poisoned at the end of the turn
  ASSERT_EQ(target_end_of_turn.where1().teammate(1, 0).getStatusAilment(), AIL_NV_POISON_TOXIC);
}


TEST_F(ToxicOrbTest, ToxicOrbNotConsumed) {
  // Magikarp should still have the item
  ASSERT_TRUE(target_end_of_turn.where1().teammate(1, 0).hasItem());
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
  ASSERT_EQ(target_end_of_turn.where1().teammate(1, 0).getStatusAilment(), AIL_NV_NONE);
}

class ToxicOrbOverwriteTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("mew"))
          .addMove(pokedex_->move("will-o-wisp"))
          .setLevel(100));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magikarp"))
          .setInitialItem(pokedex_->item("toxic orb"))
          .addMove(pokedex_->move("splash"))
          .setLevel(100));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    auto target_untouched = engine_->initialState();

    // Mew uses Will-O-Wisp (index 0). Magikarp uses Splash (index 0).
    auto turn_result = engine_->updateState(
        target_untouched, Action::move(0), Action::moveAlly(0, 0));
    target_end_of_turn = turn_result;
  }

  PossibleEnvironments target_end_of_turn;
};

TEST_F(ToxicOrbOverwriteTest, DoesNotOverwriteBurn) {
  // Magikarp should be burned and NOT badly poisoned at the end of the turn
  ASSERT_EQ(target_end_of_turn.where1Status(0).teammate(1, 0).getStatusAilment(), AIL_NV_BURN);
}
