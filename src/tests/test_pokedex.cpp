#include <gtest/gtest.h>
#include <iostream>

#include "pokemonai/pokedex_static.h"
#include "pokemonai/pokedex_dynamic.h"

#include "pokemonai/team_nonvolatile.h"


class PokedexGen4Test : public ::testing::Test {
 protected:
  std::shared_ptr<Pokedex> pokedex;

  void validateCounts(const Pokedex& pkdex) {
    EXPECT_EQ(pkdex.getAbilities().size(), 123);
    EXPECT_EQ(pkdex.getItems().size(), 97);
    EXPECT_EQ(pkdex.getNatures().size(), 26);
    EXPECT_EQ(pkdex.getMoves().size(), 384);
    EXPECT_EQ(pkdex.getPokemon().size(), 505);
    EXPECT_EQ(pkdex.getTypes().size(), 18);
    // TODO - count total number of ability, item, move, etc. plugins. Not just
    //  engine plugins
    EXPECT_GE(pkdex.getExtensions().getNumPlugins(), 10);
  }
};


class PokedexGen4StaticTest : public PokedexGen4Test {
 protected:
  void SetUp() override {
#ifndef GEN4_STATIC
    GTEST_SKIP() << "Generation-4 Test";
#endif

    pokedex = std::make_shared<PokedexStatic>();
  }
};


class PokedexGen4DynamicTest : public PokedexGen4Test {
 protected:
  void SetUp() override {
    PokedexDynamic::Config cfg;
    cfg.prefixPath_ = "data/gen4/";
    cfg.pluginsPath_ = "build/scripts/gen4/";

    pokedex = std::make_shared<PokedexDynamic>(cfg);
  }
};


TEST_F(PokedexGen4StaticTest, LoadsItems) { validateCounts(*pokedex); }


TEST_F(PokedexGen4DynamicTest, LoadsItems) { validateCounts(*pokedex); }


TEST_F(PokedexGen4StaticTest, PrintsTeamWithoutCrashing) {
  auto team = TeamNonVolatile::load("teams/hexTeamA.txt");
  team.printSummary(std::cout);
}


TEST_F(PokedexGen4DynamicTest, PrintsTeamWithoutCrashing) {
  auto team = TeamNonVolatile::load("teams/hexTeamA.txt");
  team.printSummary(std::cout);
}


class PokedexGen1Test : public ::testing::Test {
 protected:
  void SetUp() override { pokedex = std::make_shared<PokedexStatic>(); }

  std::shared_ptr<Pokedex> pokedex;

  void validateCounts(const Pokedex& pkdex) {
    EXPECT_EQ(pkdex.getAbilities().size(), 0);
    EXPECT_EQ(pkdex.getItems().size(), 0);
    EXPECT_EQ(pkdex.getNatures().size(), 0);
    EXPECT_EQ(pkdex.getMoves().size(), 165);
    EXPECT_EQ(pkdex.getPokemon().size(), 151);
    EXPECT_EQ(pkdex.getTypes().size(), 16);
    EXPECT_GE(pkdex.getExtensions().getNumPlugins(), 10);
  }
};


class PokedexGen1StaticTest : public PokedexGen1Test {
 protected:
  void SetUp() override {
#ifndef GEN1_STATIC
    GTEST_SKIP() << "Generation-1 Test";
#endif

    PokedexStatic::Config cfg;
    cfg.prefixPath_ = "data/gen1/";

    pokedex = std::make_shared<PokedexStatic>(cfg);
  }
};


TEST_F(PokedexGen1StaticTest, LoadsItems) { validateCounts(*pokedex); }


class PokedexGen1DynamicTest : public PokedexGen1Test {
 protected:
  void SetUp() override {
    PokedexDynamic::Config cfg;
    cfg.prefixPath_ = "data/gen1/";
    cfg.pluginsPath_ = "build/scripts/gen1/";

    pokedex = std::make_shared<PokedexDynamic>(cfg);
  }
};


TEST_F(PokedexGen1DynamicTest, LoadsItems) { validateCounts(*pokedex); }
