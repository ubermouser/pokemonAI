#include "engine_test.hpp"

class IsValidActionTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("charmander"))
            .addMove(pokedex_->move("cut")))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("squirtle"))
            .addMove(pokedex_->move("surf")));
    // clang-format on
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);
  }
};

class IsValidSwapTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();
    // clang-format off
    auto team = TeamNonVolatile()
            .addPokemon(PokemonNonVolatile()
                .setBase(pokedex_->pokemon("torkoal"))
                .addMove(pokedex_->move("explosion")))
            .addPokemon(PokemonNonVolatile()
                .setBase(pokedex_->pokemon("squirtle"))
                .addMove(pokedex_->move("surf")));
    // clang-format on
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


class BasicEngineTest : public Gen4EngineTest {
 protected:
  void SetUp() override {
    Gen4EngineTest::SetUp();

    // clang-format off
    auto team = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("charmander"))
            .addMove(pokedex_->move("cut"))
            .addMove(pokedex_->move("fire blast"))
            .addMove(pokedex_->move("swords dance")))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("aipom"))
            .addMove(pokedex_->move("screech")));
    // clang-format on
    auto environment = EnvironmentNonvolatile(team, team, true);
    engine_->setEnvironment(environment);
  }
};


class BasicEngine2v2Test : public BasicEngineTest {
 protected:
  void SetUp() override {
#if USE_LEGACY_ENGINE
    GTEST_SKIP() << "Neo-Engine test";
#endif

    BasicEngineTest::SetUp();

    engine_->setNumActivePokemon(2);
    engine_->initialize();
  }
};


class GetValidActionsTest : public BasicEngineTest {};

class GetValidActions2v2Test : public BasicEngine2v2Test {};


TEST_F(IsValidActionTest, Basic) {
  // pokemon may move freely when both are alive:
  EXPECT_TRUE(
      engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::move(0)));
}


TEST_F(IsValidActionTest, MoveInvalid) {
  // Move index out of bounds (charmander has only 1 move)
  EXPECT_EQ(
      engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::move(1))
          .reason,
      IsValidResult::MOVE_INVALID);
}


TEST_F(IsValidActionTest, MoveNoPP) {
  auto state = engine_->initialState();
  auto data = state.data();
  data.teams[0].teammates[0].actions[0].PPcurrent = 0;
  ConstEnvironmentVolatile noPPState(state.nv(), data);

  EXPECT_EQ(
      engine_->isValidAction(noPPState, Actor(TEAM_A, 0), Action::move(0)).reason,
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
      engine_->isValidAction(allyDeadState, Actor(TEAM_A, 0), Action::moveAlly(0, 1))
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
              engine_->initialState(), Actor(TEAM_A, 0), Action::moveAlly(0, 0))
          .reason,
      IsValidResult::MOVE_FRIENDLY_TARGET_SELF);
}


TEST_F(IsValidSwapTest, SwitchInvalidPokemon) {
  // Team has 2 pokemon, cannot switch to index 3
  EXPECT_EQ(
      engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::swap(3))
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
      engine_->isValidAction(allyDeadState, Actor(TEAM_A, 0), Action::swap(1)).reason,
      IsValidResult::SWITCH_POKEMON_DEAD);
}


TEST_F(IsValidActionTest, WaitNotAllowed) {
  // Wait not allowed if opponent is alive
  EXPECT_EQ(
      engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::wait())
          .reason,
      IsValidResult::WAIT_NOT_ALLOWED);
}


TEST_F(IsValidActionTest, StruggleNotAllowed) {
  // Struggle not allowed if moves still have PP
  EXPECT_EQ(
      engine_
          ->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::struggle())
          .reason,
      IsValidResult::STRUGGLE_NOT_ALLOWED);
}


TEST_F(IsValidActionTest, StruggleAllowed) {
  auto state = engine_->initialState();
  auto data = state.data();
  data.teams[0].teammates[0].actions[0].PPcurrent = 0;
  ConstEnvironmentVolatile noPPState(state.nv(), data);

  // Struggle IS allowed if all moves have 0 PP
  EXPECT_TRUE(engine_->isValidAction(noPPState, Actor(TEAM_A, 0), Action::struggle()));
  // Struggle is the only valid move action:
  EXPECT_EQ(
      engine_->getValidMoveActions(noPPState, Actor(TEAM_A, 0)).size(), 1);
}


TEST_F(IsValidActionTest, ActionTypeDisabled) {
  // Undefined action type (e.g. Action::MOVE_UNDEFINED which is 0)
  EXPECT_EQ(
      engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action()).reason,
      IsValidResult::ACTION_TYPE_DISABLED);
}


TEST_F(IsValidActionTest, InvalidFriendlyTarget) {
  // Attempt to target a friendly pokemon that doesn't exist (index 1 when team
  // size is 1) This hits the `action.iFriendly() >= cTV.nv().getNumTeammates()`
  // check for MOVE_0..3
  EXPECT_EQ(
      engine_
          ->isValidAction(
              engine_->initialState(), Actor(TEAM_A, 0), Action::moveAlly(0, 1))
          .reason,
      IsValidResult::INVALID_FRIENDLY_TARGET);
}


TEST_F(IsValidActionTest, MoveActorNotActive) {
  // Actor(TEAM_A, 1) is Squirtle, who is NOT active (Charmander is 0)
  auto state = engine_->initialState();
  EXPECT_EQ(
      engine_->isValidAction(state, Actor(TEAM_A, 1), Action::move(0)).reason,
      IsValidResult::MOVE_ACTOR_NOT_ACTIVE);
}


TEST_F(IsValidSwapTest, SwitchActivePokemon) {
  // Team A: 0 is active, 1 is bench.
  // Actor(TEAM_A, 1) is bench. Trying to swap to 0 (which is active).
  auto state = engine_->initialState();
  EXPECT_EQ(
      engine_->isValidAction(state, Actor(TEAM_A, 1), Action::swap(0)).reason,
      IsValidResult::SWITCH_ACTIVE_POKEMON);
}


TEST_F(IsValidSwapTest, ActivePokemonChanged) {
  // active pokemon has changed after a swap turn:
  EXPECT_EQ(engine_->initialState().getTeam(0).getICPKV(), 0);
  EXPECT_EQ(swap_squirtle.where1().getTeam(0).getICPKV(), 1);
}

TEST_F(IsValidSwapTest, SwapSelf) {
  // pokemon may not swap to themselves:
  EXPECT_FALSE(
      engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::swap(0)));
  EXPECT_FALSE(
      engine_->isValidAction(swap_squirtle.where1(), Actor(TEAM_A, 1), Action::swap(1)));
}

TEST_F(IsValidSwapTest, SwapLiving) {
  // pokemon may swap when alive if teammate is also alive:
  EXPECT_TRUE(
      engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::swap(1)));
  EXPECT_TRUE(
      engine_->isValidAction(engine_->initialState(), Actor(TEAM_B, 0), Action::swap(1)));
}

TEST_F(IsValidSwapTest, SwapFromDeadActive) {
  // dead pokemon may swap:
  EXPECT_TRUE(
      engine_->isValidAction(torkoal_dead.where1(), Actor(TEAM_B, 0), Action::swap(1)));
}

TEST_F(IsValidSwapTest, SwapEnemyDead) {
  // living pokemon may NOT swap when the enemy is dead (standard engine rules):
  EXPECT_FALSE(
      engine_->isValidAction(torkoal_dead.where1(), Actor(TEAM_A, 1), Action::swap(0)));
  EXPECT_TRUE(
      engine_->isValidAction(torkoal_dead.where1(), Actor(TEAM_A, 1), Action::wait()));
}

TEST_F(IsValidSwapTest, SwapBothDead) {
  // if BOTH pokemon are dead, both pokemon may swap:
  EXPECT_TRUE(
      engine_->isValidAction(both_dead.where1(), Actor(TEAM_A, 1), Action::swap(0)));
  EXPECT_TRUE(
      engine_->isValidAction(both_dead.where1(), Actor(TEAM_B, 0), Action::swap(1)));
}

TEST_F(IsValidSwapTest, ValidActionsCount) {
  // move counts should be accurate:
  EXPECT_EQ(
      engine_->getValidActions(torkoal_dead.where1().getEnv(), Actor(TEAM_B, 0)).size(),
      1);
}


TEST_F(IsValidSwapTest, MoveTargetDead) {
  // Team B's active pokemon is dead, Team A squirtle cannot target it
  EXPECT_EQ(
      engine_->isValidAction(torkoal_dead.where1(), Actor(TEAM_A, 1), Action::move(0))
          .reason,
      IsValidResult::MOVE_TARGET_DEAD);
}


TEST_F(IsValidSwapTest, MoveSelfDead) {
  // Team B's active pokemon (torkoal) is dead, it cannot move
  EXPECT_EQ(
      engine_->isValidAction(torkoal_dead.where1(), Actor(TEAM_B, 0), Action::move(0))
          .reason,
      IsValidResult::MOVE_SELF_DEAD);
}


TEST_F(GetValidActionsTest, AllActionsActiveTeammate) {
  EXPECT_EQ(
      engine_->getValidActions(engine_->initialState(), {TEAM_A, 0}).size(), 4);
}


TEST_F(GetValidActionsTest, AllMoveActions) {
  EXPECT_EQ(
      engine_->getValidMoveActions(engine_->initialState(), {TEAM_A, 0}).size(),
      3);
}


TEST_F(GetValidActionsTest, AllSwapActions) {
  EXPECT_EQ(
      engine_->getValidSwapActions(engine_->initialState(), {TEAM_A, 0}).size(),
      1);
}


TEST_F(GetValidActionsTest, AllValidActionMaps) {
  auto maps = engine_->getAllValidActions(engine_->initialState(), TEAM_A);
  // Charmander has 3 valid move actions and 1 valid swap action.
  EXPECT_EQ(maps.size(), 4);
  for (const auto& map : maps) {
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.count(Actor(TEAM_A, 0)));
  }
}


TEST_F(GetValidActions2v2Test, Activates2PokemonPerTeam) {
  auto state = engine_->initialState();
  EXPECT_EQ(state.getNumActivePokemon(), 4);
}


TEST_F(GetValidActions2v2Test, AllValidActionMaps) {
  auto maps = engine_->getAllValidActions(engine_->initialState(), TEAM_A);
  // Charmander has 3 valid move actions and 1 valid swap action.
  EXPECT_EQ(maps.size(), 8);
  for (const auto& map : maps) {
    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.count(Actor(TEAM_A, 0)));
    EXPECT_TRUE(map.count(Actor(TEAM_A, 1)));
  }
}


TEST_F(BasicEngineTest, PrimaryHitAndCrit) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());  // cut

  result.printStates();
  EXPECT_EQ(result.size(), 3);
  EXPECT_EQ(result.where1Hit(0).hasHit(0), true);
  EXPECT_EQ(result.where1Miss(0).hasHit(0), false);
  EXPECT_EQ(result.where1Crit(0).hasCrit(0), true);
}


TEST_F(BasicEngineTest, PrimaryHitAgainstSwap) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      Action::move(0),
      Action::swap(1));  // cut vs swap

  result.printStates();
  auto state = result.where1Hit(0);
  // swapped-out pokemon takes no damage
  EXPECT_EQ(state.teammate(TEAM_B, 0).getPercentHP(), 1.);
  // swapped-in pokemon takes damage
  EXPECT_LT(state.teammate(TEAM_B, 1).getPercentHP(), 1.);
  // swapped-in pokemon is active
  EXPECT_TRUE(state.teammate(TEAM_B, 1).isActive());
  EXPECT_FALSE(state.teammate(TEAM_B, 0).isActive());
}


TEST_F(BasicEngineTest, Swap) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::swap(1), Action::wait());

  result.printStates();
  auto state = result.where1Switch(0);
  // swapped-in pokemon is active
  EXPECT_TRUE(state.teammate(TEAM_A, 1).isActive());
  EXPECT_FALSE(state.teammate(TEAM_A, 0).isActive());
}


TEST_F(BasicEngineTest, PrimaryHitStatusAndCrit) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(1), Action::wait());  // fire blast

  result.printStates();
  EXPECT_EQ(result.size(), 5);
  EXPECT_EQ(result.whereMiss(0).size(), 1);
  EXPECT_EQ(result.whereHit(0).size(), 4);
  EXPECT_EQ(result.whereHitNoCrit(0).size(), 2);
  EXPECT_EQ(result.whereHitNoStatus(0).size(), 2);
  EXPECT_EQ(result.whereCrit(0).size(), 2);
  EXPECT_EQ(result.whereStatus(0).size(), 2);
}


TEST_F(BasicEngineTest, SpeedTie) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::move(0));  // cut vs cut

  result.printStates();
  EXPECT_EQ(result.size(), 18);
  EXPECT_EQ(result.getNumUnique(), 9);
  EXPECT_EQ(result.whereHit(0).size(), 6);
  EXPECT_EQ(result.whereMiss(0).size(), 3);
  EXPECT_EQ(result.whereCrit(0).size(), 3);
}


TEST_F(BasicEngineTest, BuffStat) {
  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      Action::move(2),  // swords dance
      Action::wait());

  result.printStates();
  // swords dance has --- P.Accuracy and 100 S.Accuracy, so it should always hit
  // and status.
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result.where1().hasHit(0), true);
  EXPECT_EQ(result.where1().hasSecondary(0), true);
  EXPECT_EQ(result.where1().teammate(0, 0).getBoost(FV_ATTACK), 2);
}


TEST_F(BasicEngineTest, DebuffStat) {
  auto team_a = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("aipom"))
          .addMove(pokedex_->move("screech")));
  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("squirtle"))
          .addMove(pokedex_->move("tail whip")));
  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  result.printStates();
  // screech has 85 S.Accuracy, so it can hit or miss.
  // It has --- P.Accuracy.
  EXPECT_EQ(result.size(), 2);

  auto hitState = result.where1Hit(0);
  EXPECT_EQ(hitState.hasHit(0), true);
  EXPECT_EQ(hitState.hasSecondary(0), true);
  EXPECT_EQ(hitState.teammate(1, 0).getBoost(FV_DEFENSE), -2);

  auto missState = result.where1([](const ConstEnvironmentPossible& state) {
    return !state.hasSecondary(0);
  });
  EXPECT_EQ(missState.hasHit(0), true);
  EXPECT_EQ(missState.hasSecondary(0), false);
  EXPECT_EQ(missState.teammate(1, 0).getBoost(FV_DEFENSE), 0);
}


TEST_F(BasicEngineTest, StatusHitAndMiss) {
  auto team_a = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("arbok"))
          .addMove(pokedex_->move("glare")));
  auto team_b = TeamNonVolatile().addPokemon(
      PokemonNonVolatile()
          .setBase(pokedex_->pokemon("charmander"))
          .addMove(pokedex_->move("cut")));
  auto environment = EnvironmentNonvolatile(team_a, team_b, true);
  engine_->setEnvironment(environment);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(), Action::move(0), Action::wait());

  result.printStates();
  // Glare has 75% accuracy and no damage/crit.
  // It should produce two states: hit and miss.
  EXPECT_EQ(result.size(), 2);

  auto hit_state = result.at(0);
  EXPECT_EQ(
      hit_state.getTeam(1).teammate(0).getStatusAilment(), AIL_NV_PARALYSIS);

  auto miss_state = result.at(1);
  EXPECT_EQ(miss_state.getTeam(1).teammate(0).getStatusAilment(), AIL_NV_NONE);
}


TEST_F(BasicEngineTest, HighEngineAccuracyWithSpeedTie) {
  engine_->setAccuracy(16);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      Action::move(1),
      Action::move(1));  // two fire blasts

  EXPECT_EQ(result.size(), 8450);
  EXPECT_EQ(result.getNumUnique(), 49);
  result.printStates();
}


TEST_F(BasicEngineTest, HighEngineAccuracySingleMove) {
  engine_->setAccuracy(16);

  PossibleEnvironments result = engine_->updateState(
      engine_->initialState(),
      Action::move(1),
      Action::wait());  // fire blast

  EXPECT_EQ(result.size(), 65);
  EXPECT_EQ(result.getNumUnique(), 7);
  result.printStates();
}


TEST_F(BasicEngineTest, HighEvasionAndAccuracy) {
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
