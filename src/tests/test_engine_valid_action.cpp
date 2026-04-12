#include <fmt/ranges.h>
#include <gtest/gtest.h>
#include <spdlog/common.h>

#include "mock_engine_test.hpp"
#include "pokemonai/engine.h"
#include "pokemonai/pkCU.h"

// 1v1 Combined Validation Tests
class IsValidAction1v1Test : public MockEngineTest {
 protected:
  void SetUp() override {
    spdlog::set_level(spdlog::level::trace);
    MockEngineTest::SetUp();
    // clang-format off
    auto teamA = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .addMove(pokedex_->move("move_any_adjacent"))  // 0
            .addMove(pokedex_->move("move_explosion"))     // 1
            .addMove(pokedex_->move("move_any_ally"))      // 2
            .addMove(pokedex_->move("move_suicide")))      // 3
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon2"))
            .addMove(pokedex_->move("move_any_adjacent"))  // 0
            .addMove(pokedex_->move("move_suicide")))      // 1
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .addMove(pokedex_->move("move_any_ally_self"))); // 0

    auto teamB = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon3"))
            .addMove(pokedex_->move("move_any_adjacent"))  // 0
            .addMove(pokedex_->move("move_explosion"))     // 1
            .addMove(pokedex_->move("move_faint"))         // 2
            .addMove(pokedex_->move("move_zero_pp")))      // 3
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon4"))
            .addMove(pokedex_->move("move_any_adjacent"))  // 0
            .addMove(pokedex_->move("move_explosion")));   // 1
    // clang-format on
    auto environment = EnvironmentNonvolatile(teamA, teamB, true);
    engine_->setEnvironment(environment);
  }

  PossibleEnvironments setup_TA0_zeroPP() {
    // Setup zero-PP state: Team B uses move_zero_pp (index 3) on Team A's
    // active Use move(3) for both to avoid killing the opponent before PP is
    // zeroed.
    return engine_->updateState(
        engine_->initialState(), Action::wait(), Action::move(3));
  }

  PossibleEnvironments setup_TA1_vs_TB0() {
    // Setup swapped teammate state
    return engine_->updateState(
        engine_->initialState(), Action::swap(1), Action::wait());
  }

  PossibleEnvironments setup_TA0_dead() {
    // Setup dead active using move_suicide (move index 3)
    return engine_->updateState(
        engine_->initialState(), Action::move(3), Action::wait());
  }

  PossibleEnvironments setup_TA0_active_TA1_dead() {
    // Setup dead on bench: switch to 1, suicide 1, replace with 0
    // Pkmn 1 has suicide at index 1
    auto state1 = setup_TA1_vs_TB0();
    auto state2 = engine_->updateState(
        state1.where1().getEnv(), Action::move(1), Action::wait());
    return engine_->updateState(
        state2.where1().getEnv(),
        {{Actor(TEAM_A, 0), Action::activate()},
         {Actor(TEAM_B, 0), Action::wait()}});
  }

  PossibleEnvironments setup_both_dead() {
    // Both dead: Team A's pkmn 0 and 1 faint via explosion
    return engine_->updateState(
        engine_->initialState(), Action::move(1), Action::move(1));
  }

  void setup_blockMoveEnv() {
    auto teamA = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .setAbility(pokedex_->ability("ability_block_move"))
            .addMove(pokedex_->move("move_any_adjacent")));

    auto teamB = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon3"))
            .addMove(pokedex_->move("move_any_adjacent")));

    auto env_nv = EnvironmentNonvolatile(teamA, teamB, true);
    engine_->setEnvironment(env_nv);
  }

  void setup_blockSwapEnv() {
    auto teamA = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .setAbility(pokedex_->ability("ability_block_swap"))
            .addMove(pokedex_->move("move_any_adjacent")))
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon2"))
            .addMove(pokedex_->move("move_any_adjacent")));

    auto teamB = TeamNonVolatile()
        .addPokemon(PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon3"))
            .addMove(pokedex_->move("move_any_adjacent")));

    auto env_nv = EnvironmentNonvolatile(teamA, teamB, true);
    engine_->setEnvironment(env_nv);
  }
};

// --- Basic Validation Tests ---

TEST_F(IsValidAction1v1Test, Basic) {
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::move(0)));
}

TEST_F(IsValidAction1v1Test, BasicTargeted) {
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::moveEnemy(0, 0)));
}

TEST_F(IsValidAction1v1Test, MoveInvalid) {
  // Move index out of bounds for Pokemon 1 (which only has 2 moves)
  EXPECT_EQ(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 1), Action::move(2)).reason,
            IsValidResult::MOVE_INVALID);
}

TEST_F(IsValidAction1v1Test, MoveNoPP) {
  auto zero_pp_result = setup_TA0_zeroPP();
  EXPECT_EQ(engine_->isValidAction(zero_pp_result.where1().getEnv(), Actor(TEAM_A, 0), Action::move(0)).reason, IsValidResult::MOVE_NO_PP);
}

TEST_F(IsValidAction1v1Test, WaitNotAllowed) {
  EXPECT_EQ(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::wait()).reason,
            IsValidResult::WAIT_NOT_ALLOWED);
}

TEST_F(IsValidAction1v1Test, StruggleNotAllowed) {
  EXPECT_EQ(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::struggle()).reason,
            IsValidResult::STRUGGLE_NOT_ALLOWED);
}

TEST_F(IsValidAction1v1Test, StruggleAllowed) {
  auto zero_pp_result = setup_TA0_zeroPP();
  EXPECT_TRUE(engine_->isValidAction(zero_pp_result.where1().getEnv(), Actor(TEAM_A, 0), Action::struggle()));
  EXPECT_EQ(engine_->getValidMoveActions(zero_pp_result.where1().getEnv(), Actor(TEAM_A, 0)).size(), 1);
}

TEST_F(IsValidAction1v1Test, ActionTypeDisabled) {
  EXPECT_EQ(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action()).reason,
            IsValidResult::ACTION_TYPE_DISABLED);
}

TEST_F(IsValidAction1v1Test, MoveTargetNotActive) {
  auto state = engine_->initialState();
  EXPECT_EQ(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::moveEnemy(0, 1)).reason,
            IsValidResult::MOVE_TARGET_NOT_ACTIVE);
}

TEST_F(IsValidAction1v1Test, MoveActorNotActive) {
  auto state = engine_->initialState();
  EXPECT_EQ(engine_->isValidAction(state, Actor(TEAM_A, 1), Action::move(0)).reason,
            IsValidResult::MOVE_ACTOR_NOT_ACTIVE);
}

// --- Listing Tests ---

TEST_F(IsValidAction1v1Test, AllActionsActiveTeammate) {
  auto actions = engine_->getValidActions(engine_->initialState(), {TEAM_A, 0});

  fmt::print("{}\n", fmt::streamed(actions));
  EXPECT_EQ(actions.size(), 7);  // (5 move actions + 2 swaps)
}


TEST_F(IsValidAction1v1Test, AllMoveActions) {
  auto actions =
      engine_->getValidMoveActions(engine_->initialState(), {TEAM_A, 0});

  fmt::print("{}\n", fmt::streamed(actions));
  EXPECT_EQ(actions.size(), 5);  // (1+1+2+1)
}


TEST_F(IsValidAction1v1Test, AllSwapActions) {
  auto actions =
      engine_->getValidSwapActions(engine_->initialState(), {TEAM_A, 0});

  fmt::print("{}\n", fmt::streamed(actions));
  EXPECT_EQ(actions.size(), 2);
}


TEST_F(IsValidAction1v1Test, AllValidActionMaps) {
  auto maps = engine_->getAllValidActions(engine_->initialState(), TEAM_A);

  fmt::print("{}\n", fmt::streamed(maps));
  for (const auto& map : maps) {
    EXPECT_EQ(map.size(), 1);
    EXPECT_TRUE(map.count(Actor(TEAM_A, 0)));
  }
  EXPECT_EQ(maps.size(), 7);  // (5 moves + 2 swaps)
}


TEST_F(IsValidAction1v1Test, AllValidActionMapsWhenSwapping) {
  auto state = setup_TA0_dead();
  auto maps = engine_->getAllValidActions(state.where1(), TEAM_A);

  fmt::print("{}\n", fmt::streamed(maps));
  for (const auto& map : maps) { EXPECT_EQ(map.size(), 1); }
  EXPECT_EQ(maps.size(), 2);  // 2 activations
}

// --- Swap Tests ---

TEST_F(IsValidAction1v1Test, SwitchInvalidPokemon) {
  EXPECT_EQ(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::swap(3)).reason,
            IsValidResult::SWITCH_INVALID_POKEMON);
}


TEST_F(IsValidAction1v1Test, SwitchPokemonDead) {
  // Try switching to pkmn 1 which is dead on bench in bench_dead state
  auto bench_dead = setup_TA0_active_TA1_dead();

  auto result = engine_->isValidAction(
      bench_dead.where1().getEnv(), Actor(TEAM_A, 0), Action::swap(1));
  EXPECT_EQ(result.reason, IsValidResult::SWITCH_POKEMON_FAINTED);
}


TEST_F(IsValidAction1v1Test, SwitchActivePokemon) {
  auto result = engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 1), Action::swap(0));
  EXPECT_EQ(result.reason, IsValidResult::SWITCH_ACTOR_NOT_ACTIVE);
}


TEST_F(IsValidAction1v1Test, ActivePokemonChanged) {
  EXPECT_TRUE(engine_->initialState().teammate(TEAM_A, 0).isActive());
  EXPECT_FALSE(engine_->initialState().teammate(TEAM_A, 1).isActive());

  auto swap_teammate = setup_TA1_vs_TB0();
  EXPECT_FALSE(swap_teammate.where1().teammate(TEAM_A, 0).isActive());
  EXPECT_TRUE(swap_teammate.where1().teammate(TEAM_A, 1).isActive());
}


TEST_F(IsValidAction1v1Test, SwapSelf) {
  EXPECT_FALSE(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::swap(0)));
}


TEST_F(IsValidAction1v1Test, SwapLiving) {
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Actor(TEAM_A, 0), Action::swap(1)));
  EXPECT_TRUE(engine_->isValidAction(engine_->initialState(), Actor(TEAM_B, 0), Action::swap(1)));
}


TEST_F(IsValidAction1v1Test, ActivateWhenReplacementNeeded) {
  auto active_dead = setup_TA0_dead();

  // Actor 0 is dead and deactivated. Actor 1 must activate itself.
  EXPECT_TRUE(engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_A, 1), Action::activate()));
}


TEST_F(IsValidAction1v1Test, CanOnlyWaitWhenReplacementNeeded) {
  auto active_dead = setup_TA0_dead();

  // clang-format off
  EXPECT_TRUE(engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_B, 0), Action::wait()));
  EXPECT_EQ(engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_B, 0), Action::move(0)).reason,
      IsValidResult::REPLACEMENT_NEEDED);
  EXPECT_EQ(engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_B, 0), Action::struggle()).reason,
      IsValidResult::REPLACEMENT_NEEDED);
  EXPECT_EQ(engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_B, 0), Action::swap(1)).reason,
      IsValidResult::REPLACEMENT_NEEDED);
  // clang-format on
}


TEST_F(IsValidAction1v1Test, CannotWaitWhenMustReplace) {
  auto active_dead = setup_TA0_dead();

  auto result = engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_A, 0), Action::wait());
  EXPECT_EQ(result.reason, IsValidResult::MOVE_ACTOR_NOT_ACTIVE);
}


TEST_F(IsValidAction1v1Test, SwapEnemyDead) {
  auto active_dead = setup_TA0_dead();

  EXPECT_FALSE(engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_B, 0), Action::swap(1)));
  EXPECT_TRUE(engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_B, 0), Action::wait()));
}


TEST_F(IsValidAction1v1Test, SwapBothDead) {
  auto both_dead = setup_both_dead();

  // Both teams have deactivated actors. Replacements must activate themselves.
  EXPECT_TRUE(engine_->isValidAction(
      both_dead.where1().getEnv(), Actor(TEAM_A, 1), Action::activate()));
  EXPECT_TRUE(engine_->isValidAction(
      both_dead.where1().getEnv(), Actor(TEAM_B, 1), Action::activate()));
}


TEST_F(IsValidAction1v1Test, ValidEntryActionsCount) {
  auto active_dead = setup_TA0_dead();
  auto actions =
      engine_->getValidEntryActions(active_dead.where1().getEnv(), TEAM_A);

  fmt::print("{}", fmt::streamed(actions));
  EXPECT_EQ(actions.size(), 2);  // (both pkmn 1 and 2 can enter play)
}


TEST_F(IsValidAction1v1Test, MoveTargetDead) {
  auto active_dead = setup_TA0_active_TA1_dead();

  auto result = engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_B, 0), Action::moveEnemy(0, 1));
  EXPECT_EQ(result.reason, IsValidResult::MOVE_TARGET_NOT_ACTIVE);
}


TEST_F(IsValidAction1v1Test, MoveSelfDead) {
  auto active_dead = setup_TA0_active_TA1_dead();

  auto result = engine_->isValidAction(
      active_dead.where1().getEnv(), Actor(TEAM_A, 1), Action::move(0));
  EXPECT_EQ(result.reason, IsValidResult::MOVE_ACTOR_NOT_ACTIVE);
}

// --- Ally Tests ---

TEST_F(IsValidAction1v1Test, MoveFriendlyTargetDead) {
  auto bench_dead = setup_TA0_active_TA1_dead();
  auto result = engine_->isValidAction(
      bench_dead.where1().getEnv(), Actor(TEAM_A, 0), Action::moveAlly(2, 1));
  // Pkmn 1 is dead on bench.
  EXPECT_EQ(result.reason, IsValidResult::MOVE_TARGET_FAINTED);
}


TEST_F(IsValidAction1v1Test, MoveFriendlyTargetSelf) {
  auto result = engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveAlly(2, 0));
  EXPECT_EQ(result.reason, IsValidResult::MOVE_FRIENDLY_TARGET_SELF);
}


TEST_F(IsValidAction1v1Test, MoveFriendlyTargetSelfAllowed) {
  auto state = engine_->updateState(
      engine_->initialState(), Action::swap(2), Action::wait());
  EXPECT_TRUE(engine_->isValidAction(
      state.where1().getEnv(), Actor(TEAM_A, 2), Action::moveAlly(0, 2)));
}


TEST_F(IsValidAction1v1Test, InvalidFriendlyTarget) {
  auto result = engine_->isValidAction(
      engine_->initialState(), Actor(TEAM_A, 0), Action::moveAlly(2, 3));
  EXPECT_EQ(result.reason, IsValidResult::INVALID_TARGET);
}

// --- Script Restriction Tests ---

TEST_F(IsValidAction1v1Test, ScriptRestrictions_MoveBlockedByAbility) {
  setup_blockMoveEnv();
  auto state = engine_->initialState();
  auto result = engine_->isValidAction(state, Actor(TEAM_A, 0), Action::move(0));
  EXPECT_EQ(result.reason, IsValidResult::MOVE_LOCKED_BY_SCRIPT);
}


TEST_F(IsValidAction1v1Test, ScriptRestrictions_StruggleAllowedWhenMovesBlocked) {
  setup_blockMoveEnv();
  auto state = engine_->initialState();
  // Struggle should be allowed because the only other move is locked
  EXPECT_TRUE(engine_->isValidAction(state, Actor(TEAM_A, 0), Action::struggle()));
}


TEST_F(IsValidAction1v1Test, ScriptRestrictions_SwapBlockedByAbility) {
  setup_blockSwapEnv();
  auto state = engine_->initialState();
  // Test switching from lead to teammate 1
  auto result = engine_->isValidAction(state, Actor(TEAM_A, 0), Action::swap(1));
  EXPECT_EQ(result.reason, IsValidResult::SWITCH_LOCKED_BY_SCRIPT);
}

// --- Action Validation Mode Tests ---


TEST_F(IsValidAction1v1Test, ActionValidation_FULL_ThrowsOnInvalidWait) {
  engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::FULL);
  ActionMap actions = {
      {Actor(TEAM_A, 0), Action::wait()}, {Actor(TEAM_B, 0), Action::wait()}};
  EXPECT_THROW(
      engine_->updateState(engine_->initialState(), actions),
      std::invalid_argument);
}


TEST_F(IsValidAction1v1Test, ActionValidation_NONE_PermitsInvalidWait) {
  engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::NONE);
  // A2 and B1 aren't even active, so their actions can't be valid
  ActionMap actions = {
      {Actor(TEAM_A, 2), Action::wait()}, {Actor(TEAM_B, 1), Action::wait()}};
  EXPECT_NO_THROW(engine_->updateState(engine_->initialState(), actions));
}


TEST_F(IsValidAction1v1Test, ActionValidation_WAIT_PermitsWaitWhenMidgame) {
  engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::WAIT_ONLY);
  ActionMap actions = {
      {Actor(TEAM_A, 0), Action::wait()}, {Actor(TEAM_B, 0), Action::wait()}};
  EXPECT_NO_THROW(engine_->updateState(engine_->initialState(), actions));
}


TEST_F(IsValidAction1v1Test, ActionValidation_WAIT_PermitsWaitWhenEnemyDead) {
  engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::WAIT_ONLY);
  auto state = setup_TA0_dead();  // TA0 is dead
  // Team B is fine, they can wait normally anyway, but let's be explicit
  ActionMap actions = {
      {Actor(TEAM_A, 1), Action::activate()},
      {Actor(TEAM_B, 0), Action::wait()}};
  EXPECT_NO_THROW(engine_->updateState(state.where1().getEnv(), actions));
}


TEST_F(IsValidAction1v1Test, ActionValidation_WAIT_ThrowsOnWaitWhenSelfDead) {
  engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::WAIT_ONLY);
  auto state = setup_TA0_dead();  // TA0 is dead, Team A MUST activate 1 or 2
  // If Team A tries to WAIT instead of activate
  ActionMap actions = {
      {Actor(TEAM_A, 1), Action::wait()}, {Actor(TEAM_B, 0), Action::wait()}};
  EXPECT_THROW(
      engine_->updateState(state.where1().getEnv(), actions),
      std::invalid_argument);
}


TEST_F(IsValidAction1v1Test, ActionValidation_WAIT_ThrowsOnOtherInvalidMoves) {
  engine_->setAllowInvalidMoves(NeoPkCU::ActionValidationMethod::WAIT_ONLY);
  auto zero_pp_result = setup_TA0_zeroPP();
  // Actor 0 has 0 PP on move 0. Trying to MOVE(0) should still throw.
  ActionMap actions = {
      {Actor(TEAM_A, 0), Action::move(0)}, {Actor(TEAM_B, 0), Action::wait()}};
  EXPECT_THROW(
      engine_->updateState(zero_pp_result.where1().getEnv(), actions),
      std::invalid_argument);
}
