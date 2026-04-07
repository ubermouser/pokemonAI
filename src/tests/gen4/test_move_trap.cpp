#include "engine_test.hpp"

class TrapTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
  }

  void setupState(const std::string& moveA, const std::string& moveB,
                  const std::string& itemA = "", const std::string& itemB = "",
                  std::string speciesA = "", std::string speciesB = "charmander") {

    std::string abilityA = "torrent";
    if (speciesA.empty()) {
        if (moveA == "mean look") { speciesA = "gastly"; abilityA = "levitate"; }
        else if (moveA == "block") { speciesA = "bronzong"; abilityA = "levitate"; }
        else if (moveA == "spider web") { speciesA = "spinarak"; abilityA = "swarm"; }
        else speciesA = "squirtle";
    } else {
        if (speciesA == "crobat") abilityA = "inner focus";
        else if (speciesA == "gastly") abilityA = "levitate";
        else if (speciesA == "bronzong") abilityA = "levitate";
        else if (speciesA == "spinarak") abilityA = "swarm";
    }

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
          .setAbility(pokedex_->ability("blaze")) // Charmander default
          .addMove(pokedex_->move(moveB))
          .setLevel(100);
    if (!itemB.empty()) {
        pokeB.setInitialItem(pokedex_->item(itemB));
    }

    auto team_a = TeamNonVolatile()
        .addPokemon(pokeA)
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("bulbasaur"))
          .setAbility(pokedex_->ability("overgrow"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(pokeB)
        .addPokemon(PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle")) // Replaced Pidgey (Keen Eye not impl)
          .setAbility(pokedex_->ability("torrent"))
          .addMove(pokedex_->move("tackle"))
          .setLevel(100));

    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));
  }
};

TEST_F(TrapTest, MeanLookTraps) {
  // Team A: Gastly (Mean Look) vs Team B: Charmander (Growl)
  setupState("mean look", "growl");

  // Turn 1: Team A uses Mean Look
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto state = turn1.where1();

  // Team B tries to switch (Action::swap(1)) - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(state.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));

  // Team A (User) should be able to switch
  EXPECT_TRUE(engine_->isValidAction(state.getEnv(), Actor(TEAM_A, 0), Action::swap(1)));
}

TEST_F(TrapTest, BlockTraps) {
  // Team A: Bronzong (Block) vs Team B: Charmander (Growl)
  setupState("block", "growl");

  // Turn 1: Team A uses Block
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto state = turn1.where1();

  // Team B tries to switch - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(state.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));
}

TEST_F(TrapTest, SpiderWebTraps) {
  // Team A: Spinarak (Spider Web) vs Team B: Charmander (Growl)
  setupState("spider web", "growl");

  // Turn 1: Team A uses Spider Web
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto state = turn1.where1();

  // Team B tries to switch - Should be invalid
  EXPECT_FALSE(engine_->isValidAction(state.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));
}

TEST_F(TrapTest, ShedShellEscapesTrap) {
  // Team A: Gastly (Mean Look) vs Team B: Charmander (Growl) + Shed Shell
  setupState("mean look", "growl", "", "shed shell");

  // Turn 1: Team A uses Mean Look
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto state = turn1.where1();

  // Team B has Shed Shell, so switching should be valid despite trap
  EXPECT_TRUE(engine_->isValidAction(state.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));
}

TEST_F(TrapTest, TrapClearedOnUserSwitch) {
  // Team A: Gastly (Mean Look) vs Team B: Charmander (Growl)
  setupState("mean look", "growl");

  // Turn 1: Team A uses Mean Look
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto state1 = turn1.where1();

  // Verify trapped
  EXPECT_FALSE(engine_->isValidAction(state1.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));

  // Turn 2: Team A switches out (Manual switch)
  auto turn2 = engine_->updateState(state1, Action::swap(1), Action::move(0));
  auto state2 = turn2.where1();

  // Trap should be cleared. Team B can switch now.
  EXPECT_TRUE(engine_->isValidAction(state2.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));
}

TEST_F(TrapTest, TrapClearedOnUTurn) {
  // Team A: Crobat (Mean Look, U-turn) vs Team B: Charmander (Growl)
  auto pokeA = PokemonNonVolatile()
        .setBase(pokedex_->pokemon("crobat"))
        .setAbility(pokedex_->ability("inner focus"))
        .addMove(pokedex_->move("mean look"))
        .addMove(pokedex_->move("u-turn"))
        .setLevel(100);

  auto pokeB = PokemonNonVolatile()
        .setBase(pokedex_->pokemon("charmander"))
        .setAbility(pokedex_->ability("blaze"))
        .addMove(pokedex_->move("growl"))
        .setLevel(100);

  auto team_a = TeamNonVolatile()
      .addPokemon(pokeA)
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("bulbasaur"))
        .addMove(pokedex_->move("tackle")));

  auto team_b = TeamNonVolatile()
      .addPokemon(pokeB)
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("squirtle"))
        .addMove(pokedex_->move("tackle")));

  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

  // Turn 1: Team A uses Mean Look
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::move(0));
  auto state1 = turn1.where1();

  // Verify trapped
  EXPECT_FALSE(engine_->isValidAction(state1.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));

  // Turn 2: Team A uses U-turn (Action::move(1))
  auto turn2 = engine_->updateState(state1, Action::moveAlly(1, 1), Action::move(0));
  auto state2 = turn2.where1();

  // Team A should have switched (Crobat -> Bulbasaur).
  EXPECT_EQ(state2.getTeam(TEAM_A).getICPKV(), 1);

  // Trap should be cleared. Team B can switch now.
  EXPECT_TRUE(engine_->isValidAction(state2.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));
}

TEST_F(TrapTest, TrapClearedOnUserDeath) {
  // Team A: Gastly (Mean Look) vs Team B: Zapdos (Thunderbolt)
  auto pokeA = PokemonNonVolatile()
        .setBase(pokedex_->pokemon("gastly"))
        .setAbility(pokedex_->ability("levitate"))
        .addMove(pokedex_->move("mean look"))
        .setLevel(1); // Die easily

  auto pokeB = PokemonNonVolatile()
        .setBase(pokedex_->pokemon("zapdos"))
        .setAbility(pokedex_->ability("pressure"))
        .addMove(pokedex_->move("thunderbolt"))
        .addMove(pokedex_->move("roost")) // Valid move for Zapdos
        .setLevel(100);

  auto team_a = TeamNonVolatile()
      .addPokemon(pokeA)
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("bulbasaur"))
        .addMove(pokedex_->move("tackle")));

  auto team_b = TeamNonVolatile()
      .addPokemon(pokeB)
      .addPokemon(PokemonNonVolatile()
        .setBase(pokedex_->pokemon("squirtle"))
        .addMove(pokedex_->move("tackle")));

  engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));

  // Turn 1: A uses Mean Look, B uses Roost (to let A live and trap)
  auto turn1 = engine_->updateState(engine_->initialState(), Action::move(0), Action::moveAlly(1, 0));
  auto state1 = turn1.where1();

  // Verify trapped
  EXPECT_FALSE(engine_->isValidAction(state1.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));

  // Turn 2: A uses Mean Look, B uses Thunderbolt (kills A)
  auto turn2 = engine_->updateState(state1, Action::move(0), Action::move(0));
  auto state2 = turn2.where1();

  // Check that Gastly (Active pokemon of Team A) is dead
  EXPECT_FALSE(state2.teammate(TEAM_A, 0).isAlive());

  // A is dead. We must switch in.
  auto turn3 = engine_->updateState(state2, Action::swap(1), Action::wait());
  auto state3 = turn3.where1();

  EXPECT_EQ(state3.getTeam(TEAM_A).getICPKV(), 1); // Bulbasaur in

  // Trap should be cleared. Team B can switch now.
  EXPECT_TRUE(engine_->isValidAction(state3.getEnv(), Actor(TEAM_B, 0), Action::swap(1)));
}
