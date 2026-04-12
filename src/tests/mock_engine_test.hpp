#ifndef MOCK_ENGINE_TEST_HPP
#define MOCK_ENGINE_TEST_HPP

#include <gtest/gtest.h>

#include <memory>

#include "mock_pokedex.hpp"
#include "pokemonai/engine.h"

class MockEngineTest : public ::testing::Test {
 protected:
  static void resetPluginCalls() { plugin_calls.fill(0); }

  void SetUp() override {
    spdlog::set_level(spdlog::level::trace);

    resetPluginCalls();
    pokedex_ = std::make_shared<MockPokedex>();
    pokedex_->setAllowInvalidPokemon(true);
    pokedex_->setAllowInvalidTeams(true);

    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::WAIT_ONLY);
  }

  std::shared_ptr<MockPokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
  EnvironmentNonvolatile environment_nv;
};

#endif // MOCK_ENGINE_TEST_HPP
