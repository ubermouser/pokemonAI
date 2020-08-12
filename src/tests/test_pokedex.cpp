#include <gtest/gtest.h>
#include <iostream>

#include <inc/pokedex_static.h>
#include <inc/pokedex_dynamic.h>
#include <inc/pkIO.h>

#include "inc/team_nonvolatile.h"

class PokedexTest : public ::testing::Test {
protected:
  void SetUp() override {
    pokedex = std::make_shared<PokedexDynamic>();
  }

  std::shared_ptr<PokedexDynamic> pokedex;
};

void validateGen4Counts(const Pokedex& pkdex) {
  EXPECT_EQ(pkdex.getAbilities().size(), 123);
  EXPECT_EQ(pkdex.getItems().size(), 97);
  EXPECT_EQ(pkdex.getNatures().size(), 26);
  EXPECT_EQ(pkdex.getMoves().size(), 383);
  EXPECT_EQ(pkdex.getPokemon().size(), 505);
  EXPECT_EQ(pkdex.getTypes().size(), 18);
}

TEST(StaticPokedexTest, LoadsGen4Items) {
  PokedexStatic pkdex;
  validateGen4Counts(pkdex);
  EXPECT_EQ(pkdex.getExtensions().getNumPlugins(), 0);
}

TEST(DynamicPokedexTest, LoadsGen4Items) {
  PokedexDynamic pkdex;
  validateGen4Counts(pkdex);
  EXPECT_GE(pkdex.getExtensions().getNumPlugins(), 50);
}

TEST_F(PokedexTest, PrintsTeamWithoutCrashing) {
  const TeamNonVolatile& team = PkIO::inputPlayerTeam("teams/hexTeamA.txt");
  team.output(std::cout);
}