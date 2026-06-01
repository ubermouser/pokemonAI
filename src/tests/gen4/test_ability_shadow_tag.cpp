#include "engine_test.hpp"

class ShadowTagTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
  }

  void setupState(
      const std::string& abilityA,
      const std::string& abilityB,
      const std::string& itemA = "",
      const std::string& itemB = "",
      const std::string& speciesA = "squirtle",
      const std::string& speciesB = "wobbuffet",
      const std::string& moveA = "",
      const std::string& moveB = "") {
    std::string actualMoveA =
        moveA.empty() ? ((speciesA == "wobbuffet") ? "splash" : "tackle")
                      : moveA;
    std::string actualMoveB =
        moveB.empty() ? ((speciesB == "wobbuffet") ? "splash" : "tackle")
                      : moveB;

    // clang-format off
    auto pokeA = PokemonNonVolatile()
          .setBase(pokedex_->pokemon(speciesA))
          .setAbility(pokedex_->ability(abilityA))
          .addMove(pokedex_->move(actualMoveA))
          .setLevel(100);
    if (!itemA.empty()) {
        pokeA.setInitialItem(pokedex_->item(itemA));
    }

    auto pokeB = PokemonNonVolatile()
          .setBase(pokedex_->pokemon(speciesB))
          .setAbility(pokedex_->ability(abilityB))
          .addMove(pokedex_->move(actualMoveB))
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
    // clang-format on

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));
  }
};


TEST_F(ShadowTagTest, Trapped) {
  // Team A: Squirtle (Torrent) vs Team B: Wobbuffet (Shadow Tag)
  setupState("torrent", "shadow tag");
  auto state = engine_->initialState();

  // Team A tries to switch (Action::swap(1)) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::swap(1)));

  // Team B (Shadow Tag user) should be able to switch
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::swap(1)));
}


TEST_F(ShadowTagTest, MirrorMatch) {
  // Team A: Wobbuffet (Shadow Tag) vs Team B: Wobbuffet (Shadow Tag)
  setupState("shadow tag", "shadow tag", "", "", "wobbuffet", "wobbuffet");
  auto state = engine_->initialState();

  // Team A tries to switch - Should be valid because they also have Shadow Tag
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::swap(1)));
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_B, 0), Action::swap(1)));
}


TEST_F(ShadowTagTest, ShedShell) {
  // Team A: Squirtle (Torrent) + Shed Shell vs Team B: Wobbuffet (Shadow Tag)
  setupState("torrent", "shadow tag", "shed shell");
  auto state = engine_->initialState();

  // Team A tries to switch - Should be valid because of Shed Shell
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::swap(1)));
}


TEST_F(ShadowTagTest, Normal) {
  // Team A: Squirtle (Torrent) vs Team B: Squirtle (Torrent)
  setupState("torrent", "torrent", "", "", "squirtle", "squirtle");
  auto state = engine_->initialState();

  // Team A tries to switch - Should be valid
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::swap(1)));
}


TEST_F(ShadowTagTest, FaintedPokemonCanSwitch) {
  // Team A: Gengar (Levitate) vs Team B: Wobbuffet (Shadow Tag)
  // Gengar uses Explosion to faint itself
  setupState(
      "levitate", "shadow tag", "", "", "gengar", "wobbuffet", "explosion");

  auto turn1 = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());
  auto state = turn1.where1();

  // Gengar should be fainted
  EXPECT_FALSE(state.teammate(TEAM_A, 0).isAlive());
  EXPECT_TRUE(state.teammate(TEAM_B, 0).isAlive());

  // The bench pokemon should be able to activate itself because the active
  // pokemon is fainted, even though Wobbuffet has Shadow Tag
  EXPECT_TRUE(engine_->isValidAction(
      state.getEnv(), Actor(TEAM_A, 1), Action::activate()));
}
