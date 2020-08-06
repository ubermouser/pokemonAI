#include <gtest/gtest.h>
#include <iostream>

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

TEST_F(PokedexTest, LoadsGen4Items) {
  EXPECT_EQ(pokedex->getAbilities().size(), 123);
  EXPECT_EQ(pokedex->getItems().size(), 97);
  EXPECT_EQ(pokedex->getNatures().size(), 26);
  EXPECT_EQ(pokedex->getMoves().size(), 383);
  EXPECT_EQ(pokedex->getPokemon().size(), 505);
  EXPECT_EQ(pokedex->getTypes().size(), 18);
}

TEST_F(PokedexTest, PrintsTeamWithoutCrashing) {
  const TeamNonVolatile& team = PkIO::inputPlayerTeam("teams/hexTeamA.txt");
  team.output(std::cout);
}