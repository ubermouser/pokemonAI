#include "engine_test.hpp"

class IsValidActionTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    auto team = TeamNonVolatile().addPokemon(
        PokemonNonVolatile()
            .setBase(pokedex_->pokemon("charmander"))
            .addMove(pokedex_->move("cut")));
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);
  }
};

class IsValidSwapTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    auto team =
        TeamNonVolatile()
            .addPokemon(PokemonNonVolatile()
                            .setBase(pokedex_->pokemon("torkoal"))
                            .addMove(pokedex_->move("explosion")))
            .addPokemon(PokemonNonVolatile()
                            .setBase(pokedex_->pokemon("squirtle"))
                            .addMove(pokedex_->move("surf")));
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);

    swap_squirtle = engine_->updateState(
        engine_->initialState(), Action::swap(1), Action::wait());
    torkoal_dead = engine_->updateState(
        swap_squirtle.where1(), Action::move(0), Action::wait());
    both_dead = engine_->updateState(
        swap_squirtle.where1(), Action::wait(), Action::move(0));
  }

  PossibleEnvironments swap_squirtle;
  PossibleEnvironments torkoal_dead;
  PossibleEnvironments both_dead;
};


TEST_F(IsValidActionTest, Basic) {
  // pokemon may move freely when both are alive:
  EXPECT_TRUE(
      engine_->isValidAction(engine_->initialState(), Action::move(0), TEAM_A));
}


TEST_F(IsValidActionTest, MoveInvalid) {
  // Move index out of bounds (charmander has only 1 move)
  EXPECT_EQ(
      engine_->isValidAction(engine_->initialState(), Action::move(1), TEAM_A)
          .reason,
      IsValidResult::MOVE_INVALID);
}


TEST_F(IsValidActionTest, MoveNoPP) {
  auto state = engine_->initialState();
  auto data = state.data();
  data.teams[0].teammates[0].actions[0].PPcurrent = 0;
  ConstEnvironmentVolatile noPPState(state.nv(), data);

  EXPECT_EQ(
      engine_->isValidAction(noPPState, Action::move(0), TEAM_A).reason,
      IsValidResult::MOVE_NO_PP);
}


TEST_F(IsValidActionTest, MoveFriendlyTargetDead) {
  auto team =
      TeamNonVolatile()
          .addPokemon(
              PokemonNonVolatile()
                  .setBase(pokedex_->pokemon("aipom"))
                  .addMove(pokedex_->move(
                      "baton pass")))  // Baton pass targets ally in this engine
          .addPokemon(PokemonNonVolatile()
                          .setBase(pokedex_->pokemon("squirtle"))
                          .addMove(pokedex_->move("surf")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto state = engine_->initialState();
  auto data = state.data();
  // Faint squirtle
  data.teams[0].teammates[1].HPcurrent = 0;
  ConstEnvironmentVolatile allyDeadState(state.nv(), data);

  // Softboiled targeting fainted squirtle
  EXPECT_EQ(
      engine_->isValidAction(allyDeadState, Action::moveAlly(0, 1), TEAM_A)
          .reason,
      IsValidResult::MOVE_FRIENDLY_TARGET_DEAD);
}


TEST_F(IsValidActionTest, MoveFriendlyTargetSelf) {
  auto team = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("aipom"))
          .addMove(pokedex_->move("baton pass")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  // Softboiled targeting self (if it's defined to target ally only, but here
  // let's assume MOVE_FRIENDLY_TARGET_SELF check) According to isValidAction:
  // doAllowMove[VALID_MOVE_FRIENDLY_IS_OTHER] = action.iFriendly() !=
  // cTV.getICPKV();
  EXPECT_EQ(
      engine_
          ->isValidAction(
              engine_->initialState(), Action::moveAlly(0, 0), TEAM_A)
          .reason,
      IsValidResult::MOVE_FRIENDLY_TARGET_SELF);
}


TEST_F(IsValidActionTest, SwitchInvalidPokemon) {
  // Team has only 1 pokemon, cannot switch to index 1
  EXPECT_EQ(
      engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_A)
          .reason,
      IsValidResult::SWITCH_INVALID_POKEMON);
}


TEST_F(IsValidActionTest, SwitchPokemonDead) {
  auto team =
      TeamNonVolatile()
          .addPokemon(PokemonNonVolatile()
                          .setBase(pokedex_->pokemon("charmander"))
                          .addMove(pokedex_->move("cut")))
          .addPokemon(PokemonNonVolatile()
                          .setBase(pokedex_->pokemon("squirtle"))
                          .addMove(pokedex_->move("surf")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  auto state = engine_->initialState();
  auto data = state.data();
  // Faint squirtle
  data.teams[0].teammates[1].HPcurrent = 0;
  ConstEnvironmentVolatile allyDeadState(state.nv(), data);

  EXPECT_EQ(
      engine_->isValidAction(allyDeadState, Action::swap(1), TEAM_A).reason,
      IsValidResult::SWITCH_POKEMON_DEAD);
}


TEST_F(IsValidActionTest, WaitNotAllowed) {
  // Wait not allowed if opponent is alive
  EXPECT_EQ(
      engine_->isValidAction(engine_->initialState(), Action::wait(), TEAM_A)
          .reason,
      IsValidResult::WAIT_NOT_ALLOWED);
}


TEST_F(IsValidActionTest, Struggle) {
  // Struggle not allowed if moves still have PP
  EXPECT_EQ(
      engine_
          ->isValidAction(engine_->initialState(), Action::struggle(), TEAM_A)
          .reason,
      IsValidResult::STRUGGLE_NOT_ALLOWED);

  // Struggle IS allowed if all moves have 0 PP
  auto state = engine_->initialState();
  auto data = state.data();
  data.teams[0].teammates[0].actions[0].PPcurrent = 0;
  ConstEnvironmentVolatile noPPState(state.nv(), data);
  EXPECT_TRUE(engine_->isValidAction(noPPState, Action::struggle(), TEAM_A));
}


TEST_F(IsValidActionTest, ActionTypeDisabled) {
  // Undefined action type (e.g. Action::MOVE_UNDEFINED which is 0)
  EXPECT_EQ(
      engine_->isValidAction(engine_->initialState(), Action(), TEAM_A).reason,
      IsValidResult::ACTION_TYPE_DISABLED);
}


TEST_F(IsValidActionTest, InvalidFriendlyTarget) {
  // Attempt to target a friendly pokemon that doesn't exist (index 1 when team
  // size is 1) This hits the `action.iFriendly() >= cTV.nv().getNumTeammates()`
  // check for MOVE_0..3
  EXPECT_EQ(
      engine_
          ->isValidAction(
              engine_->initialState(), Action::moveAlly(0, 1), TEAM_A)
          .reason,
      IsValidResult::INVALID_FRIENDLY_TARGET);
}


TEST_F(IsValidSwapTest, ActivePokemonChanged) {
  // active pokemon has changed after a swap turn:
  EXPECT_EQ(engine_->initialState().getTeam(0).getICPKV(), 0);
  EXPECT_EQ(swap_squirtle.where1().getTeam(0).getICPKV(), 1);
}

TEST_F(IsValidSwapTest, SwapSelf) {
  // pokemon may not swap to themselves:
  EXPECT_FALSE(
      engine_->isValidAction(engine_->initialState(), Action::swap(0), TEAM_A));
  EXPECT_FALSE(
      engine_->isValidAction(swap_squirtle.where1(), Action::swap(1), TEAM_A));
}

TEST_F(IsValidSwapTest, SwapLiving) {
  // pokemon may swap when alive if teammate is also alive:
  EXPECT_TRUE(
      engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_A));
  EXPECT_TRUE(
      engine_->isValidAction(engine_->initialState(), Action::swap(1), TEAM_B));
}

TEST_F(IsValidSwapTest, SwapFromDeadActive) {
  // dead pokemon may swap:
  EXPECT_TRUE(
      engine_->isValidAction(torkoal_dead.where1(), Action::swap(1), TEAM_B));
}

TEST_F(IsValidSwapTest, SwapEnemyDead) {
  // living pokemon may NOT swap when the enemy is dead (standard engine rules):
  EXPECT_FALSE(
      engine_->isValidAction(torkoal_dead.where1(), Action::swap(0), TEAM_A));
  EXPECT_TRUE(
      engine_->isValidAction(torkoal_dead.where1(), Action::wait(), TEAM_A));
}

TEST_F(IsValidSwapTest, SwapBothDead) {
  // if BOTH pokemon are dead, both pokemon may swap:
  EXPECT_TRUE(
      engine_->isValidAction(both_dead.where1(), Action::swap(0), TEAM_A));
  EXPECT_TRUE(
      engine_->isValidAction(both_dead.where1(), Action::swap(1), TEAM_B));
}

TEST_F(IsValidSwapTest, ValidActionsCount) {
  // move counts should be accurate:
  EXPECT_EQ(
      engine_->getValidActions(torkoal_dead.where1().getEnv(), TEAM_B).size(),
      1);
}


TEST_F(IsValidSwapTest, MoveTargetDead) {
  // Team B's active pokemon is dead, Team A squirtle cannot target it
  EXPECT_EQ(
      engine_->isValidAction(torkoal_dead.where1(), Action::move(0), TEAM_A)
          .reason,
      IsValidResult::MOVE_TARGET_DEAD);
}


TEST_F(IsValidSwapTest, MoveSelfDead) {
  // Team B's active pokemon (torkoal) is dead, it cannot move
  EXPECT_EQ(
      engine_->isValidAction(torkoal_dead.where1(), Action::move(0), TEAM_B)
          .reason,
      IsValidResult::MOVE_SELF_DEAD);
}


TEST_F(Gen4EngineTest, PrimaryHitAndCrit) {
  auto team = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("cut")));
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.where1Hit(0).hasHit(0), true);
  EXPECT_EQ(result.where1Miss(0).hasHit(0), false);
  EXPECT_EQ(result.where1Crit(0).hasCrit(0), true);
}


TEST_F(Gen4EngineTest, HighEngineAccuracy) {
  // moves with extremely high numbers of branches might cause stack probability
  // that sums less than 1
  engine_->setAccuracy(16);
  auto team = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move(
              "fire blast")));  // move can hit, crit, status, and miss
  auto environment = EnvironmentNonvolatile(team, team, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));

  EXPECT_EQ(result.size(), 8450);
  EXPECT_EQ(result.getNumUnique(), 49);
  result.printStates();
}


TEST_F(Gen4EngineTest, HighEvasionAndAccuracy) {
  // Reproduce branchProbability > FixType(0) assertion failure
  engine_->setAccuracy(16);
  auto team_a = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("mud-slap")));

  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("smeargle"))
          .addMove(pokedex_->move("sweet scent")));

  auto environment_nv =
      std::make_shared<EnvironmentNonvolatile>(team_a, team_b, true);
  engine_->setEnvironment(environment_nv);

  PossibleEnvironments results = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));
  auto currentState = results.where1Hit(TEAM_A);
  results.printStates();

  // Run 7 turns of accuracy/evasion debuffs
  for (int turn = 1; turn <= 7; ++turn) {
    std::cout << "Turn " << turn << ": Mud-slap vs Sweet Scent" << std::endl;
    // We want to ensure Mud-slap hits to keep the debuff loop going.
    // results.whereHit(TEAM_A) returns states where Team A's move hit.
    results =
        engine_->updateState(currentState, Action::move(0), Action::move(0));
    results.printStates();

    ASSERT_FALSE(results.empty())
        << "Engine returned no states on turn " << turn;

    // will throw if Mud-slap fails to hit
    currentState = results.where1Hit(TEAM_A);
  }

  std::cout << "Successfully completed 7 turns of Mud-slap vs Sweet Scent"
            << std::endl;
}
