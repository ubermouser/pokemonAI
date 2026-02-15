#include "engine_test.hpp"

class TrappingTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
  }

  void setupState(const std::string& abilityA, const std::string& abilityB,
                  const std::string& itemA = "", const std::string& itemB = "",
                  const std::string& speciesA = "squirtle", const std::string& speciesB = "wobbuffet") {

    auto getMove = [&](const std::string& species) {
        if (species == "wobbuffet") return "splash";
        if (species == "skarmory") return "aerial ace";
        if (species == "bronzor") return "tackle";
        if (species == "gastly") return "confuse ray";
        if (species == "pidgey") return "aerial ace";
        if (species == "dugtrio") return "aerial ace";
        if (species == "magnemite") return "discharge";
        if (species == "charizard") return "ember";
        if (species == "empoleon") return "surf";
        return "tackle";
    };

    std::string moveA = getMove(speciesA);
    std::string moveB = getMove(speciesB);

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
        pokeB.setInitialItem(itemB == "---" ? *Item::no_item : pokedex_->item(itemB));
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

// --- Arena Trap Tests ---

TEST_F(TrappingTest, ArenaTrap_GroundedTrapped) {
  // Team A: Squirtle (Torrent) vs Team B: Dugtrio (Arena Trap)
  setupState("torrent", "arena trap", "", "", "squirtle", "dugtrio");
  auto state = engine_->initialState();
  EXPECT_FALSE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(TrappingTest, ArenaTrap_FlyingNotTrapped) {
  // Team A: Charizard (Blaze) vs Team B: Dugtrio (Arena Trap)
  setupState("blaze", "arena trap", "", "", "charizard", "dugtrio");
  auto state = engine_->initialState();
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(TrappingTest, ArenaTrap_LevitateNotTrapped) {
  // Team A: Bronzor (Levitate) vs Team B: Dugtrio (Arena Trap)
  setupState("levitate", "arena trap", "", "", "bronzor", "dugtrio");
  auto state = engine_->initialState();
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(TrappingTest, ArenaTrap_ShedShellBypass) {
  setupState("torrent", "arena trap", "shed shell", "", "squirtle", "dugtrio");
  auto state = engine_->initialState();
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

// --- Magnet Pull Tests ---

TEST_F(TrappingTest, MagnetPull_SteelTrapped) {
  // Team A: Empoleon (Torrent) vs Team B: Magnemite (Magnet Pull)
  setupState("torrent", "magnet pull", "", "", "empoleon", "magnemite");
  auto state = engine_->initialState();
  EXPECT_FALSE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(TrappingTest, MagnetPull_NonSteelNotTrapped) {
  setupState("torrent", "magnet pull", "", "", "squirtle", "magnemite");
  auto state = engine_->initialState();
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(TrappingTest, MagnetPull_ShedShellBypass) {
  setupState("torrent", "magnet pull", "shed shell", "", "empoleon", "magnemite");
  auto state = engine_->initialState();
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

// --- Shadow Tag Tests ---

TEST_F(TrappingTest, ShadowTag_Trapped) {
  setupState("torrent", "shadow tag", "", "", "squirtle", "wobbuffet");
  auto state = engine_->initialState();
  EXPECT_FALSE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(TrappingTest, ShadowTag_MirrorMatchNotTrapped) {
  setupState("shadow tag", "shadow tag", "", "", "wobbuffet", "wobbuffet");
  auto state = engine_->initialState();
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(TrappingTest, ShadowTag_ShedShellBypass) {
  setupState("torrent", "shadow tag", "shed shell", "", "squirtle", "wobbuffet");
  auto state = engine_->initialState();
  EXPECT_TRUE(engine_->isValidAction(state, Action::swap(1), TEAM_A));
}

TEST_F(TrappingTest, ShadowTag_FaintedPokemonCanSwitch) {
  setupState("torrent", "shadow tag");

  auto state = engine_->initialState();
  EnvironmentVolatileData envData = state.data();
  EnvironmentVolatile envV(state.nv(), envData);
  envV.getTeam(TEAM_A).getPKV().setHP(0);

  EXPECT_TRUE(engine_->isValidAction(envV, Action::swap(1), TEAM_A));
}
