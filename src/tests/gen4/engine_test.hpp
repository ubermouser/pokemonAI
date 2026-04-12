#include <gtest/gtest.h>
#include <spdlog/common.h>

#include <memory>

#include "pokemonai/engine.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pkai.h"
#include "pokemonai/pokedex_dynamic.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/state_transition_printer.h"
#include "pokemonai/team_volatile.h"

class Gen4EngineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    initialize_logger(spdlog::level::warn);

    PokedexDynamic::Config cfg;
    cfg.prefixPath_ = "data/gen4/";
    cfg.pluginsPath_ = "build/scripts/gen4/";

    pokedex_ = std::make_shared<PokedexDynamic>(cfg);
    engine_ = std::make_shared<PkCU>();
    engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::WAIT_ONLY);

    spdlog::set_level(spdlog::level::trace);
  }

  std::shared_ptr<Pokedex> pokedex_;
  std::shared_ptr<PkCU> engine_;
  EnvironmentNonvolatile environment_nv;
};
