#include "engine_test.hpp"

class ShadowTagTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
  }

  void setupState(const std::string& abilityA, const std::string& abilityB,
                  const std::string& itemA = "", const std::string& itemB = "",
                  const std::string& speciesA = "squirtle", const std::string& speciesB = "wobbuffet") {

    std::string moveA = (speciesA == "wobbuffet") ? "splash" : "tackle";
    std::string moveB = (speciesB == "wobbuffet") ? "splash" : "tackle";

    auto pokeA = PokemonNonVolatile()
          .setBase(pokedex_->pokemon(speciesA))
          .setAbility(pokedex_->ability(abilityA))
          .addMove(pokedex_->move(moveA))
          .setLevel(100);
    if (!itemA.empty()) {
        pokeA.setInitialItem(pokedex_->item(itemA));
    }

    auto pokeB = PokemonNonVolatile()
          .setBase(pokedex_->pokemon(speciesB))
          .setAbility(pokedex_->ability(abilityB))
          .addMove(pokedex_->move(moveB))
          .setLevel(100);
    if (!itemB.empty()) {
        pokeB.setInitialItem(pokedex_->item(itemB));
    }

    auto team_a = TeamNonVolatile()
        .addPokemon(pokeA)
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .setAbility(pokedex_->ability("blaze"))
          .addMove(pokedex_->move("ember"))
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(pokeB)
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .setAbility(pokedex_->ability("blaze"))
          .addMove(pokedex_->move("growl"))
          .setLevel(100));

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));
  }
};

TEST_F(ShadowTagTest, Trapped) {
  // Team A: Squirtle (Torrent) vs Team B: Wobbuffet (Shadow Tag)
  setupState("torrent", "shadow tag");
  auto state = engine_->initialState();

  // Team A tries to switch (Action::swap(1)) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(state, Action::swap(1), TEAM_A));

  // Team B (Shadow Tag user) should be able to switch
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_B));
}

TEST_F(ShadowTagTest, MirrorMatch) {
  // Team A: Wobbuffet (Shadow Tag) vs Team B: Wobbuffet (Shadow Tag)
  setupState("shadow tag", "shadow tag", "", "", "wobbuffet", "wobbuffet");
  auto state = engine_->initialState();

  // Team A tries to switch - Should be valid because they also have Shadow Tag
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));

  // Team B tries to switch - Should be valid
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_B));
}

TEST_F(ShadowTagTest, ShedShell) {
  // Team A: Squirtle (Torrent) + Shed Shell vs Team B: Wobbuffet (Shadow Tag)
  setupState("torrent", "shadow tag", "shed shell");
  auto state = engine_->initialState();

  // Team A tries to switch - Should be valid because of Shed Shell
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(ShadowTagTest, Normal) {
  // Team A: Squirtle (Torrent) vs Team B: Squirtle (Torrent)
  setupState("torrent", "torrent", "", "", "squirtle", "squirtle");
  auto state = engine_->initialState();

  // Team A tries to switch - Should be valid
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(ShadowTagTest, FaintedPokemonCanSwitch) {
  // Team A: Squirtle (Torrent) vs Team B: Wobbuffet (Shadow Tag)
  setupState("torrent", "shadow tag");

  // Faint Team A's Squirtle
  auto state = engine_->initialState();
  EnvironmentVolatileData envData = state.data();
  EnvironmentVolatile envV(state.nv(), envData);
  envV.getTeam(TEAM_A).getPKV().setHP(0);

  // Team A should be able to switch because Squirtle is fainted, even though
  // Wobbuffet has Shadow Tag
  EXPECT_TRUE(engine_->isValidAction(envV, Action::swap(1), TEAM_A));
}
