#include "engine_test.hpp"


class FocusSashTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("garchomp"))
          .addMove(pokedex_->move("earthquake"))
          .setLevel(100))
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(5));
    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("magikarp"))
          .setInitialItem(pokedex_->item("focus sash"))
          .addMove(pokedex_->move("splash"))
          .setLevel(5));

    environment_nv = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setEnvironment(environment_nv);

    auto target_untouched = engine_->initialState();

    // 2. Bulbasaur uses Tackle, magikarp survives easily
    auto swap_state = engine_->updateState(
        target_untouched, Action::swap(1), Action::move(0));
    target_damaged = engine_->updateState(
        swap_state.where1(), Action::move(0), Action::move(0));

    // Garchomp uses earthquake, magikarp only just survives.
    target_bigdamage = engine_->updateState(
        target_untouched, Action::move(0), Action::move(0));

    // Garchump uses earthquake after bulbasaur uses tackle, magikarp dies.
    auto swap_back_to_garchomp = engine_->updateState(
        target_damaged.where1(), Action::swap(0), Action::wait());
    target_dead = engine_->updateState(
        swap_back_to_garchomp.where1(), Action::move(0), Action::move(0));
  }

  PossibleEnvironments target_damaged;
  PossibleEnvironments target_bigdamage;
  PossibleEnvironments target_dead;
};


TEST_F(FocusSashTest, FocusSashPreventsOHKO) {
  // Garchomp uses Earthquake (index 0) on full HP Magikarp
  ASSERT_EQ(target_bigdamage.where1().getEnv().getTeam(1).getPKV().getHP(), 1);
}


TEST_F(FocusSashTest, FocusSashConsumedAfterUse) {
  // Garchomp uses Earthquake (index 0) on full HP Magikarp
  ASSERT_FALSE(
      target_bigdamage.where1().getEnv().getTeam(1).getPKV().hasItem());
}


TEST_F(FocusSashTest, FocusSashDoesNotWorkIfNotFullHP) {
  // Garchomp uses Earthquake (index 0) on damaged Magikarp
  ASSERT_FALSE(target_dead.where1().getEnv().getTeam(1).getPKV().isAlive());
}


TEST_F(FocusSashTest, FocusSashReported) {
  // Garchomp uses Earthquake (index 0) on full HP Magikarp
  // We expect it to report that it used the item.
  auto output = StateTransitionPrinter::printString(
      engine_->initialState(), target_bigdamage.where1(), false);

  SCOPED_TRACE(output);
  // We expect it to report that it used the item.
  EXPECT_TRUE(output.find("magikarp used its focus sash") != std::string::npos);
}
