#include <gtest/gtest.h>
#include <spdlog/common.h>

#include <memory>

#include "pokemonai/engine.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pkai.h"
#include "pokemonai/pokedex_static.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/team_volatile.h"

class EngineTest : public ::testing::Test {
protected:
  void SetUp() override {
    verbose = 0;
    initialize_logger(spdlog::level::trace);
    pokedex_ = std::make_shared<PokedexStatic>();
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(true);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
  EnvironmentNonvolatile environment_nv;
};
