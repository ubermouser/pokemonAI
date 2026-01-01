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
  }
};


class PokedexGen4StaticTest : public PokedexGen4Test {
 protected:
  void SetUp() override { pokedex = std::make_shared<PokedexStatic>(); }
};


class PokedexGen4DynamicTest : public PokedexGen4Test {
 protected:
  void SetUp() override { pokedex = std::make_shared<PokedexDynamic>(); }
};


TEST_F(PokedexGen4StaticTest, LoadsItems) { validateCounts(*pokedex); }


// TODO(@drendleman) - test fails due to not linking correctly? Why not?
TEST_F(PokedexGen4DynamicTest, DISABLED_LoadsItems) {
  validateCounts(*pokedex);
}


TEST_F(PokedexGen4StaticTest, PrintsTeamWithoutCrashing) {
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
    EXPECT_EQ(pkdex.getTypes().size(), 15);
    EXPECT_GE(pkdex.getExtensions().getNumPlugins(), 10);
  }
};


class PokedexGen1StaticTest : public PokedexGen1Test {
 protected:
  void SetUp() override {
    PokedexStatic::Config cfg;
    cfg.prefixPath_ = "data/gen1/";

    pokedex = std::make_shared<PokedexStatic>(cfg);
  }
};


TEST_F(PokedexGen1StaticTest, DISABLED_LoadsItems) { validateCounts(*pokedex); }
