#include <gtest/gtest.h>
#include <iostream>

#include <inc/pokedex_dynamic.h>

TEST(PokedexTest, LoadsGen4Items) {
  PokedexDynamic pokedex;
  
  EXPECT_EQ(pokedex.getAbilities().size(), 123);
  EXPECT_EQ(pokedex.getItems().size(), 97);
  EXPECT_EQ(pokedex.getNatures().size(), 26);
  EXPECT_EQ(pokedex.getMoves().size(), 383);
  EXPECT_EQ(pokedex.getPokemon().size(), 505);
  EXPECT_EQ(pokedex.getTypes().size(), 18);
}
