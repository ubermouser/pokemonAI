#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <array>
#include <bitset>
#include <memory>
#include <vector>

#include "mock_engine_test.hpp"
#include "pokemonai/engine.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/team_nonvolatile.h"

// Global pointer needed by some classes

class EnginePluginTest : public MockEngineTest {
 protected:
  void SetUp() override {
    initialize_logger(spdlog::level::trace);
    MockEngineTest::SetUp();

    // clang-format off
    auto team = TeamNonVolatile()
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("test_pokemon"))
        .setAbility(pokedex_->ability("test_ability"))
        .setInitialItem(pokedex_->item("test_item"))
        .setNature(pokedex_->nature("none"))
        .addMove(pokedex_->move("test_move"))
        .addMove(pokedex_->move("status_move"))
        .addMove(pokedex_->move("move_suicide"))
        .setLevel(100))
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("test_pokemon2"))
        .setAbility(pokedex_->ability("test_ability"))
        .setInitialItem(pokedex_->item("test_item"))
        .setNature(pokedex_->nature("none"))
        .addMove(pokedex_->move("test_move"))
        .addMove(pokedex_->move("status_move"))
        .setLevel(100));
    // clang-format on

    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);
  }
};


TEST_F(EnginePluginTest, NonvolatileStateInitialization) {
  // environment_->initialize(); // called when when engine_ is initialized
  EXPECT_GT(plugin_calls[PLUGIN_ON_INIT], 0);
}


TEST_F(EnginePluginTest, IsValidActionMove) {
  engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::move(0));
  EXPECT_GT(plugin_calls[PLUGIN_ON_TESTMOVE], 0);
}


TEST_F(EnginePluginTest, IsValidActionSwap) {
  engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::swap(1));
  EXPECT_GT(plugin_calls[PLUGIN_ON_TESTSWITCH], 0);
}


TEST_F(EnginePluginTest, UpdateStateDamagingMove) {
  engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  EXPECT_GT(plugin_calls[PLUGIN_ON_SETSPEEDBRACKET], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYSPEED], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_BEGINNINGOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYACTION], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SETBASEPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYBASEPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYATTACKPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYCRITICALPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYRAWDAMAGE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SETMOVETYPE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYSTAB], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SETDEFENSETYPE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYITEMPOWER], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYHITPROBABILITY], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYCRITPROBABILITY], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_CALCULATEDAMAGE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFMOVE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYSECONDARYPROBABILITY], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SECONDARYEFFECT], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFROUND], 0);
}


TEST_F(EnginePluginTest, UpdateStateStatusMove) {
  engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());

  EXPECT_GT(plugin_calls[PLUGIN_ON_SETSPEEDBRACKET], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYSPEED], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_BEGINNINGOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYACTION], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYHITPROBABILITY], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_EVALUATEMOVE], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFROUND], 0);
  // damage effect pipeline is not called:
  EXPECT_EQ(plugin_calls[PLUGIN_ON_SETBASEPOWER], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYBASEPOWER], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYATTACKPOWER], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYCRITICALPOWER], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYRAWDAMAGE], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_SETMOVETYPE], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYSTAB], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_SETDEFENSETYPE], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYITEMPOWER], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYCRITPROBABILITY], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_CALCULATEDAMAGE], 0);
}


TEST_F(EnginePluginTest, UpdateStateSwap) {
  engine_->updateState(
      engine_->initialState(), Action::swap(1), Action::wait());


  EXPECT_EQ(plugin_calls[PLUGIN_ON_BEGINNINGOFTURN], 0);
  // EXPECT_GT(plugin_calls[PLUGIN_ON_MODIFYACTION], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SWITCHOUT], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_SWITCHIN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFTURN], 0);
  EXPECT_GT(plugin_calls[PLUGIN_ON_ENDOFROUND], 0);
  // swap actions do not have a speed:
  EXPECT_EQ(plugin_calls[PLUGIN_ON_SETSPEEDBRACKET], 0);
  // move pipeline is not called:
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYHITPROBABILITY], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_MODIFYCRITPROBABILITY], 0);
}


TEST_F(EnginePluginTest, SkipEndOfTurnIfFainted) {
  // Pokemon B (Team B) moves first (move_self), then Pokemon A (Team A) moves
  // (move_suicide)
  engine_->updateState(
      engine_->initialState(), Action::move(2), Action::wait());

  // Pokemon A moves, faints at EndOfMove, skips onEndOfTurn and onEndOfRound
  EXPECT_EQ(plugin_calls[PLUGIN_ON_BEGINNINGOFTURN], 1);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_ENDOFMOVE], 1);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_ENDOFTURN], 0);
  EXPECT_EQ(plugin_calls[PLUGIN_ON_ENDOFROUND], 0);
}
