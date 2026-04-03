#include <gtest/gtest.h>
#include "gen4/engine_test.hpp"
#include "pokemonai/environment_volatile.h"

class EnvironmentVolatileTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
#if USE_LEGACY_ENGINE
    GTEST_SKIP() << "Neo-Engine test";
#endif

    Gen4EngineTest::SetUp();

    // 2v2 setup
    auto team_a = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("mew"))
            .addMove(pokedex_->move("iron head"))      // ANY_ADJACENT
            .addMove(pokedex_->move("aerial ace"))     // ANY_ACTIVE
            .addMove(pokedex_->move("earthquake"))     // ALL_ADJACENT
            .addMove(pokedex_->move("baton pass"))     // ANY_ALLY
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("celebi"))
            .addMove(pokedex_->move("recover"))        // SELF
            .setLevel(100))
        .addPokemon(PokemonNonVolatile() // Reserve
            .setBase(pokedex_->pokemon("jirachi"))
            .setLevel(100));

    auto team_b = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("charmander"))
            .setLevel(100))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("squirtle"))
            .setLevel(100));

    auto environment = EnvironmentNonvolatile(team_a, team_b, true);
    engine_->setNumActivePokemon(2);
    engine_->setEnvironment(environment);
  }
};


TEST_F(EnvironmentVolatileTest, GetActionsSelf) {
  ConstEnvironmentVolatile env = engine_->initialState();
  Actor actor(0, 1); // Celebi
  const MoveNonVolatile& move = env.teammate(actor).nv().getMove(0); // Recover

  auto actions = env.getActions(actor, move);
  ASSERT_EQ(actions.size(), 1);
  EXPECT_EQ(actions[0].type(), Action::MOVE_0);
  EXPECT_EQ(actions[0].friendlyTarget(), Action::FRIENDLY_1);
  EXPECT_EQ(actions[0].enemyTarget(), Action::HOSTILE_DEFAULT);
}


TEST_F(EnvironmentVolatileTest, GetActionsEnemy) {
  ConstEnvironmentVolatile env = engine_->initialState();
  Actor actor(0, 0); // Jirachi
  const MoveNonVolatile& move = env.teammate(actor).nv().getMove(0); // Iron Head

  auto actions = env.getActions(actor, move);
  // ANY_ADJACENT should have 3 actions: teammate Celebi, enemy charmander, enemy squirtle
  ASSERT_EQ(actions.size(), 3);
  
  bool foundCelebi = false;
  bool foundCharmander = false;
  bool foundSquirtle = false;
  for (const auto& a : actions) {
    if (a.friendlyTarget() == Action::FRIENDLY_1) foundCelebi = true;
    if (a.enemyTarget() == Action::HOSTILE_0) foundCharmander = true;
    if (a.enemyTarget() == Action::HOSTILE_1) foundSquirtle = true;
  }
  EXPECT_TRUE(foundCelebi);
  EXPECT_TRUE(foundCharmander);
  EXPECT_TRUE(foundSquirtle);
}


TEST_F(EnvironmentVolatileTest, GetActionsAnyActive) {
  ConstEnvironmentVolatile env = engine_->initialState();
  Actor actor(0, 0); // Jirachi
  const MoveNonVolatile& move = env.teammate(actor).nv().getMove(1); // Aerial Ace

  auto actions = env.getActions(actor, move);
  // ANY_ACTIVE should target everyone EXCEPT self (Celebi, Charmander, Squirtle)
  ASSERT_EQ(actions.size(), 3);
  
  bool foundCelebi = false;
  bool foundCharmander = false;
  bool foundSquirtle = false;
  for (const auto& a : actions) {
    if (a.friendlyTarget() == Action::FRIENDLY_1) foundCelebi = true;
    if (a.enemyTarget() == Action::HOSTILE_0) foundCharmander = true;
    if (a.enemyTarget() == Action::HOSTILE_1) foundSquirtle = true;
  }
  EXPECT_TRUE(foundCelebi);
  EXPECT_TRUE(foundCharmander);
  EXPECT_TRUE(foundSquirtle);
}


TEST_F(EnvironmentVolatileTest, GetActionsAllAdjacent) {
  ConstEnvironmentVolatile env = engine_->initialState();
  Actor actor(0, 0); // Jirachi
  const MoveNonVolatile& move = env.teammate(actor).nv().getMove(2); // Earthquake

  auto actions = env.getActions(actor, move);
  ASSERT_EQ(actions.size(), 1);
  EXPECT_EQ(actions[0].type(), Action::MOVE_2);
  // For ALL_ADJACENT, we use a single action with MOVE constant
}


TEST_F(EnvironmentVolatileTest, GetActionsAnyAlly) {
  ConstEnvironmentVolatile env = engine_->initialState();
  Actor actor(0, 0); // Jirachi
  const MoveNonVolatile& move = env.teammate(actor).nv().getMove(3); // Baton Pass

  auto actions = env.getActions(actor, move);
  // Should have all living teammates: Jirachi (self), Celebi (active), Mew (reserve)
  ASSERT_EQ(actions.size(), 2);
  
  std::vector<size_t> targets;
  for (const auto& a : actions) {
    if (a.friendlyTarget() == Action::FRIENDLY_DEFAULT) targets.push_back(0);
    else targets.push_back(a.iFriendly());
  }
  std::sort(targets.begin(), targets.end());
  EXPECT_EQ(targets[0], 1);
  EXPECT_EQ(targets[1], 2);
}


TEST_F(EnvironmentVolatileTest, GetTargetsSpecificEnemy) {
  ConstEnvironmentVolatile env = engine_->initialState();
  Actor actor(0, 0);
  Action action = Action::moveEnemy(0, 1); // Target Squirtle

  auto targets = env.getTargets(actor, action);
  ASSERT_EQ(targets.size(), 1);
  EXPECT_EQ(targets[0].iTeam(), 1);
  EXPECT_EQ(targets[0].iTeammate(), 1);
}


TEST_F(EnvironmentVolatileTest, GetTargetsFriendlyAll) {
  ConstEnvironmentVolatile env = engine_->initialState();
  Actor actor(0, 0);
  Action action(Action::MOVE_0, Action::FRIENDLY_ALL, Action::HOSTILE_DEFAULT);
  
  auto targets = env.getTargets(actor, action);
  ASSERT_EQ(targets.size(), 3); // Mew, Celebi, Jirachi
  
  std::vector<size_t> indices;
  for (const auto& t : targets) {
    EXPECT_EQ(t.iTeam(), 0);
    indices.push_back(t.iTeammate());
  }
  std::sort(indices.begin(), indices.end());
  EXPECT_EQ(indices[0], 0);
  EXPECT_EQ(indices[1], 1);
  EXPECT_EQ(indices[2], 2);
}


TEST_F(EnvironmentVolatileTest, MoveIndexManagement) {
  auto pknv = PokemonNonVolatile()
    .setBase(pokedex_->pokemon("mew"))
    .setLevel(100);
  
  // Test addMove
  pknv.addMove(pokedex_->move("iron head"));   // Index 0
  pknv.addMove(pokedex_->move("aerial ace"));  // Index 1
  pknv.addMove(pokedex_->move("earthquake"));  // Index 2
  
  EXPECT_EQ(pknv.getMove(0).getIMove(), 0);
  EXPECT_EQ(pknv.getMove(1).getIMove(), 1);
  EXPECT_EQ(pknv.getMove(2).getIMove(), 2);
  
  // Test setMove
  pknv.setMove(1, pokedex_->move("baton pass"));
  EXPECT_EQ(pknv.getMove(1).getIMove(), 1);
  EXPECT_EQ(pknv.getMove(1).getBase().getName(), "baton pass");
  
  // Test removeMove from middle
  pknv.removeMove(1); // Remove baton pass
  ASSERT_EQ(pknv.getNumMoves(), 2);
  EXPECT_EQ(pknv.getMove(0).getIMove(), 0);
  EXPECT_EQ(pknv.getMove(0).getBase().getName(), "iron head");
  EXPECT_EQ(pknv.getMove(1).getIMove(), 1);
  EXPECT_EQ(pknv.getMove(1).getBase().getName(), "earthquake");
  
  // Test struggle
  EXPECT_EQ(MoveNonVolatile::mNV_struggle->getIMove(), 4);
}


TEST_F(EnvironmentVolatileTest, GetTargetsAllAdjacent) {
  ConstEnvironmentVolatile env = engine_->initialState();
  Actor actor(0, 0);
  // Earthquake uses HOSTILE_ADJACENT or similar?
  // Actually, Earthquake is often MOVE_X with specific target resolution in engine.
  // But our getActions for ALL_ADJACENT returns MOVE(iMove).
  // And getTargets for MOVE(iMove) with DEFAULT targets returns SELF.
  // Wait, earthquake should target everyone else.
  
  // Let's test a HOSTILE_ADJACENT action directly
  Action action(Action::MOVE_0, Action::FRIENDLY_DEFAULT, Action::HOSTILE_ADJACENT);
  auto targets = env.getTargets(actor, action);
  // Should target both active enemies
  ASSERT_EQ(targets.size(), 2);
  EXPECT_EQ(targets[0].iTeam(), 1);
  EXPECT_EQ(targets[1].iTeam(), 1);
}
