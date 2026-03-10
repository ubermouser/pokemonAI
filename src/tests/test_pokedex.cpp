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
    EXPECT_EQ(pkdex.getMoves().size(), 388);
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
  auto team = TeamNonVolatile::load("teams/gen4/hexTeamA.txt");
  team.printSummary(std::cout);
}


TEST_F(PokedexGen4DynamicTest, PrintsTeamWithoutCrashing) {
  auto team = TeamNonVolatile::load("teams/gen4/hexTeamA.txt");
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


TEST_F(PokedexGen4DynamicTest, AllowsInvalidMovesAndAbilities) {
  pokedex->setAllowInvalidPokemon(true);
  PokemonNonVolatile pkmn;
  pkmn.setBase(pokedex->pokemon("bulbasaur"));

  // Bulbasaur cannot normally learn "Absorb"
  EXPECT_NO_THROW(pkmn.addMove(pokedex->move("absorb")));

  // Bulbasaur cannot normally have "Blaze"
  EXPECT_NO_THROW(pkmn.setAbility(pokedex->ability("blaze")));
}


TEST_F(PokedexGen4DynamicTest, AllowsInvalidTeams) {
  pokedex->setAllowInvalidTeams(true);
  TeamNonVolatile team;
  PokemonNonVolatile pkmn1;
  pkmn1.setBase(pokedex->pokemon("bulbasaur"));
  pkmn1.setLevel(100);

  PokemonNonVolatile pkmn2;
  pkmn2.setBase(pokedex->pokemon("bulbasaur"));
  pkmn2.setLevel(100);

  EXPECT_NO_THROW(team.addPokemon(pkmn1));
  EXPECT_NO_THROW(team.addPokemon(pkmn2));
  EXPECT_EQ(team.getNumTeammates(), 2);
}
