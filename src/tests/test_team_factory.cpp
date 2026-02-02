#include "gen1/engine_test.hpp"
#include "gen4/engine_test.hpp"
#include "pokemonai/team_factory.h"


class Gen4TeamFactoryTest : public Gen4EngineTest {
 protected:
  TeamFactory factory_;

  void SetUp() override {
    Gen4EngineTest::SetUp();
    factory_.initialize(*pokedex_);
  }
};


class Gen1TeamFactoryTest : public Gen1EngineTest {
 protected:
  TeamFactory factory_;

  void SetUp() override {
    Gen1EngineTest::SetUp();
    factory_.initialize(*pokedex_);
  }
};


TEST_F(Gen4TeamFactoryTest, CreateRandom) {
  size_t teamSize = 6;
  TeamNonVolatile team = factory_.createRandom(teamSize);
  
  EXPECT_EQ(team.getNumTeammates(), teamSize);
  
  for (size_t i = 0; i < teamSize; ++i) {
    const auto& pokemon = team.teammate(i);
    EXPECT_GT(pokemon.getNumMoves(), 0);
    EXPECT_TRUE(pokemon.pokemonExists());
    
    // In Gen 4, we expect abilities and natures to exist.
    EXPECT_TRUE(pokemon.getNature().hasName());
  }
}


TEST_F(Gen1TeamFactoryTest, CreateRandom) {
  size_t teamSize = 6;
  TeamNonVolatile team = factory_.createRandom(teamSize);
  
  EXPECT_EQ(team.getNumTeammates(), teamSize);
  
  for (size_t i = 0; i < teamSize; ++i) {
    const auto& pokemon = team.teammate(i);
    EXPECT_GT(pokemon.getNumMoves(), 0);
    EXPECT_TRUE(pokemon.pokemonExists());
    
    // In Gen 1, there should be no abilities, items, or natures.
    EXPECT_FALSE(pokemon.abilityExists());
    EXPECT_FALSE(pokemon.hasInitialItem());
    EXPECT_EQ(pkdex->getNatures().size(), 0);
  }
}


TEST_F(Gen4TeamFactoryTest, Mutation) {
  TeamNonVolatile team = factory_.createRandom(1);
  TeamNonVolatile mutated = factory_.mutate(team);
  
  EXPECT_EQ(mutated.getNumTeammates(), 1);
  EXPECT_TRUE(mutated.teammate(0).pokemonExists());
  EXPECT_GT(mutated.teammate(0).getNumMoves(), 0);
}


TEST_F(Gen4TeamFactoryTest, Crossover) {
  TeamNonVolatile parentA = factory_.createRandom(6);
  TeamNonVolatile parentB = factory_.createRandom(6);
  
  TeamNonVolatile child = factory_.crossover(parentA, parentB);
  
  EXPECT_EQ(child.getNumTeammates(), 6);
  for (size_t i = 0; i < 6; ++i) {
    EXPECT_TRUE(child.teammate(i).pokemonExists());
  }
}


TEST_F(Gen1TeamFactoryTest, Mutation) {
  TeamNonVolatile team = factory_.createRandom(1);
  TeamNonVolatile mutated = factory_.mutate(team);
  
  EXPECT_EQ(mutated.getNumTeammates(), 1);
  EXPECT_TRUE(mutated.teammate(0).pokemonExists());
  EXPECT_GT(mutated.teammate(0).getNumMoves(), 0);
}


TEST_F(Gen1TeamFactoryTest, Crossover) {
  TeamNonVolatile parentA = factory_.createRandom(6);
  TeamNonVolatile parentB = factory_.createRandom(6);
  
  TeamNonVolatile child = factory_.crossover(parentA, parentB);
  
  EXPECT_EQ(child.getNumTeammates(), 6);
  for (size_t i = 0; i < 6; ++i) {
    EXPECT_TRUE(child.teammate(i).pokemonExists());
  }
}
