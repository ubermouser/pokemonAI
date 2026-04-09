#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <unordered_map>

#include "mock_engine_test.hpp"
#include "pokemonai/environment_volatile.h"


using ::testing::UnorderedElementsAreArray;


class EnvironmentVolatileTest : public MockEngineTest {
 protected:
  void SetUp() override {

    MockEngineTest::SetUp();
  }

  /**
   * @brief Helper to initialize a testing environment with a specific move on
   * the testing actor.
   *
   * @param numActive Number of active pokemon per side (1, 2, or 3).
   * @param actorSlot Internal team slot (0-5) of the pokemon to perform the
   * move.
   * @param moveName The name of the move from MockPokedex to assign to the
   * actor.
   */
  void setupEnvironment(size_t numActive, size_t actorSlot, const std::string& moveName) {
    auto createTeam = [&](size_t teamIdx, bool isActorTeam) {
      auto team = TeamNonVolatile();
      for (size_t i = 0; i < 6; ++i) {
        auto p = PokemonNonVolatile()
            .setBase(pokedex_->pokemon("test_pokemon"))
            .setLevel(100);
        if (isActorTeam && i == actorSlot) {
          p.addMove(pokedex_->move(moveName));
        } else {
          p.addMove(pokedex_->move("test_move"));
        }
        team.addPokemon(p);
      }
      return team;
    };

    auto team_a = createTeam(0, true);
    auto team_b = createTeam(1, false);
    
    engine_->setNumActivePokemon(numActive);
    engine_->setEnvironment(EnvironmentNonvolatile(team_a, team_b, true));
  }

  /**
   * @brief Comprehensive verification helper for move targeting.
   *
   * This method performs two separate validations:
   * 1. Action Generation: It calls env.getActions() and verifies that the
   * resulting set of Action objects matches @p expectedActions.
   * 2. Target Resolution: For each generated action, it calls env.getTargets()
   * and verifies that the resolved Actor list matches the pre-defined @p
   * expectedTargets.
   *
   * @param numActive Number of active pokemon per side.
   * @param actorSlot Slot of the actor performing the move.
   * @param moveName Name of the mock move being tested.
   * @param expectedActions The complete set of expected Action objects for this
   * move and board state.
   * @param expectedTargets A map from Action to its expected target Actor list.
   */
  void verify(
      size_t numActive,
      size_t actorSlot,
      const std::string& moveName,
      const std::vector<::Action>& expectedActions,
      const std::unordered_map<::Action, std::vector<Actor>>& expectedTargets) {
    setupEnvironment(numActive, actorSlot, moveName);
    auto env = engine_->initialState();
    Actor actor(0, actorSlot);
    const MoveNonVolatile& move = env.teammate(actor).nv().getMove(0);

    // Phase 1: Action Generation
    auto actions = env.getActions(actor, move);
    EXPECT_THAT(actions, UnorderedElementsAreArray(expectedActions))
        << "Failed actions for " << moveName << " [" << numActive << "v" << numActive << ", slot " << actorSlot << "]";

    // Phase 2: Target Resolution per Action
    for (const auto& action : actions) {
      auto targets = env.getTargets(actor, action);
      auto it = expectedTargets.find(action);
      if (it != expectedTargets.end()) {
        EXPECT_THAT(targets, UnorderedElementsAreArray(it->second))
            << "Failed targets for " << moveName << " action " << action;
      } else {
        ADD_FAILURE() << "No expected targets defined for action " << action << " of " << moveName;
      }
    }
  }
};


// --- 1v1 Tests ---


TEST_F(EnvironmentVolatileTest, Targeting1v1_move_self) {
  verify(1, 0, "move_self", 
         {::Action::moveAlly(0, 0)}, 
         {{::Action::moveAlly(0, 0), {Actor(0, 0)}}});
}


TEST_F(EnvironmentVolatileTest, Targeting1v1_move_any_adjacent) {
  verify(1, 0, "move_any_adjacent", 
         {::Action::moveEnemy(0, 0)}, 
         {{::Action::moveEnemy(0, 0), {Actor(1, 0)}}});
}


TEST_F(EnvironmentVolatileTest, Targeting1v1_move_any_ally) {
  verify(1, 0, "move_any_ally", 
         {::Action::moveAlly(0, 1), ::Action::moveAlly(0, 2), ::Action::moveAlly(0, 3), ::Action::moveAlly(0, 4), ::Action::moveAlly(0, 5)}, 
         {
             {::Action::moveAlly(0, 1), {Actor(0, 1)}},
             {::Action::moveAlly(0, 2), {Actor(0, 2)}},
             {::Action::moveAlly(0, 3), {Actor(0, 3)}},
             {::Action::moveAlly(0, 4), {Actor(0, 4)}},
             {::Action::moveAlly(0, 5), {Actor(0, 5)}}
         });
}


TEST_F(EnvironmentVolatileTest, Targeting1v1_move_any_ally_self) {
  // clang-format off
  verify(1, 0, "move_any_ally_self",
         {::Action::moveAlly(0, 0), ::Action::moveAlly(0, 1), ::Action::moveAlly(0, 2), ::Action::moveAlly(0, 3), ::Action::moveAlly(0, 4), ::Action::moveAlly(0, 5)},
         {
            {::Action::moveAlly(0, 0), {Actor(0, 0)}},
            {::Action::moveAlly(0, 1), {Actor(0, 1)}},
            {::Action::moveAlly(0, 2), {Actor(0, 2)}},
            {::Action::moveAlly(0, 3), {Actor(0, 3)}},
            {::Action::moveAlly(0, 4), {Actor(0, 4)}},
            {::Action::moveAlly(0, 5), {Actor(0, 5)}}
         });
  // clang-format on
}


TEST_F(EnvironmentVolatileTest, Targeting1v1_move_all_adjacent) {
  verify(1, 0, "move_all_adjacent", 
         {::Action::moveAdjacent(0)}, 
         {{::Action::moveAdjacent(0), {Actor(1, 0)}}});
}


TEST_F(EnvironmentVolatileTest, Targeting1v1_move_side_ally) {
  verify(
      1,
      0,
      "move_side_ally",
      {::Action::moveSideAlly(0)},
      {{::Action::moveSideAlly(0), {Actor(0, 0)}}});
}


// --- 2v2 Tests (Slot 0) ---


TEST_F(EnvironmentVolatileTest, Targeting2v2_Slot0_move_any_adjacent) {
  verify(2, 0, "move_any_adjacent", 
         {::Action::moveAlly(0, 1), ::Action::moveEnemy(0, 0), ::Action::moveEnemy(0, 1)}, 
         {
             {::Action::moveAlly(0, 1), {Actor(0, 1)}},
             {::Action::moveEnemy(0, 0), {Actor(1, 0)}},
             {::Action::moveEnemy(0, 1), {Actor(1, 1)}}
         });
}


TEST_F(EnvironmentVolatileTest, Targeting2v2_Slot0_move_any_ally) {
  verify(2, 0, "move_any_ally", 
         {::Action::moveAlly(0, 1), ::Action::moveAlly(0, 2), ::Action::moveAlly(0, 3), ::Action::moveAlly(0, 4), ::Action::moveAlly(0, 5)}, 
         {
             {::Action::moveAlly(0, 1), {Actor(0, 1)}},
             {::Action::moveAlly(0, 2), {Actor(0, 2)}},
             {::Action::moveAlly(0, 3), {Actor(0, 3)}},
             {::Action::moveAlly(0, 4), {Actor(0, 4)}},
             {::Action::moveAlly(0, 5), {Actor(0, 5)}}
         });
}


TEST_F(EnvironmentVolatileTest, Targeting2v2_Slot0_move_all_adjacent) {
  verify(2, 0, "move_all_adjacent", 
         {::Action::moveAdjacent(0)}, 
         {{::Action::moveAdjacent(0), {Actor(0, 1), Actor(1, 0), Actor(1, 1)}}});
}


// --- 2v2 Tests (Slot 1) ---


TEST_F(EnvironmentVolatileTest, Targeting2v2_Slot1_move_any_adjacent) {
  verify(2, 1, "move_any_adjacent", 
         {::Action::moveAlly(0, 0), ::Action::moveEnemy(0, 0), ::Action::moveEnemy(0, 1)}, 
         {
             {::Action::moveAlly(0, 0), {Actor(0, 0)}},
             {::Action::moveEnemy(0, 0), {Actor(1, 0)}},
             {::Action::moveEnemy(0, 1), {Actor(1, 1)}}
         });
}


// --- 3v3 Tests (Slot 0 - Edge) ---


TEST_F(EnvironmentVolatileTest, Targeting3v3_Slot0_move_any_adjacent) {
  verify(3, 0, "move_any_adjacent", 
         {::Action::moveAlly(0, 1), ::Action::moveEnemy(0, 0), ::Action::moveEnemy(0, 1)}, 
         {
             {::Action::moveAlly(0, 1), {Actor(0, 1)}},
             {::Action::moveEnemy(0, 0), {Actor(1, 0)}},
             {::Action::moveEnemy(0, 1), {Actor(1, 1)}}
         });
}


TEST_F(EnvironmentVolatileTest, Targeting3v3_Slot0_move_all_adjacent) {
  verify(3, 0, "move_all_adjacent", 
         {::Action::moveAdjacent(0)}, 
         {{::Action::moveAdjacent(0), {Actor(0, 1), Actor(1, 0), Actor(1, 1)}}});
}


// --- 3v3 Tests (Slot 1 - Center) ---


TEST_F(EnvironmentVolatileTest, Targeting3v3_Slot1_move_any_adjacent) {
  verify(3, 1, "move_any_adjacent", 
         {::Action::moveAlly(0, 0), ::Action::moveAlly(0, 2), ::Action::moveEnemy(0, 0), ::Action::moveEnemy(0, 1), ::Action::moveEnemy(0, 2)}, 
         {
             {::Action::moveAlly(0, 0), {Actor(0, 0)}},
             {::Action::moveAlly(0, 2), {Actor(0, 2)}},
             {::Action::moveEnemy(0, 0), {Actor(1, 0)}},
             {::Action::moveEnemy(0, 1), {Actor(1, 1)}},
             {::Action::moveEnemy(0, 2), {Actor(1, 2)}}
         });
}


// --- Comprehensive Types 3v3 (Slot 1) ---


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_any_active) {
  verify(3, 1, "move_any_active", 
         {::Action::moveAlly(0, 0), ::Action::moveAlly(0, 2), ::Action::moveEnemy(0, 0), ::Action::moveEnemy(0, 1), ::Action::moveEnemy(0, 2)}, 
         {
             {::Action::moveAlly(0, 0), {Actor(0, 0)}},
             {::Action::moveAlly(0, 2), {Actor(0, 2)}},
             {::Action::moveEnemy(0, 0), {Actor(1, 0)}},
             {::Action::moveEnemy(0, 1), {Actor(1, 1)}},
             {::Action::moveEnemy(0, 2), {Actor(1, 2)}}
         });
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_all_active) {
  verify(3, 1, "move_all_active", 
         {::Action::moveActive(0)}, 
         {{::Action::moveActive(0), {Actor(0, 0), Actor(0, 1), Actor(0, 2), Actor(1, 0), Actor(1, 1), Actor(1, 2)}}});
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_all_active_allies) {
  verify(3, 1, "move_all_active_allies", 
         {::Action::moveActiveAlly(0)}, 
         {{::Action::moveActiveAlly(0), {Actor(0, 0), Actor(0, 1), Actor(0, 2)}}});
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_all_active_enemies) {
  verify(3, 1, "move_all_active_enemies", 
         {::Action::moveActiveEnemy(0)}, 
         {{::Action::moveActiveEnemy(0), {Actor(1, 0), Actor(1, 1), Actor(1, 2)}}});
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_all_field) {
  verify(3, 1, "move_all_field", 
         {::Action::moveAll(0)}, 
         {{::Action::moveAll(0), {
             Actor(0,0), Actor(0,1), Actor(0,2), Actor(0,3), Actor(0,4), Actor(0,5),
             Actor(1,0), Actor(1,1), Actor(1,2), Actor(1,3), Actor(1,4), Actor(1,5)
         }}});
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_any_adjacent_ally) {
  verify(3, 1, "move_any_adjacent_ally", 
         {::Action::moveAlly(0, 0), ::Action::moveAlly(0, 2)}, 
         {
             {::Action::moveAlly(0, 0), {Actor(0, 0)}},
             {::Action::moveAlly(0, 2), {Actor(0, 2)}}
         });
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_any_adjacent_enemy) {
  verify(3, 1, "move_any_adjacent_enemy", 
         {::Action::moveEnemy(0, 0), ::Action::moveEnemy(0, 1), ::Action::moveEnemy(0, 2)}, 
         {
             {::Action::moveEnemy(0, 0), {Actor(1, 0)}},
             {::Action::moveEnemy(0, 1), {Actor(1, 1)}},
             {::Action::moveEnemy(0, 2), {Actor(1, 2)}}
         });
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_any_adjacent_ally_self) {
  verify(3, 1, "move_any_adjacent_ally_self", 
         {::Action::moveAlly(0, 0), ::Action::moveAlly(0, 1), ::Action::moveAlly(0, 2)}, 
         {
             {::Action::moveAlly(0, 0), {Actor(0, 0)}},
             {::Action::moveAlly(0, 1), {Actor(0, 1)}},
             {::Action::moveAlly(0, 2), {Actor(0, 2)}}
         });
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_all_adjacent_enemy) {
  verify(3, 1, "move_all_adjacent_enemy", 
         {::Action::moveAdjacentEnemy(0)}, 
         {{::Action::moveAdjacentEnemy(0), {Actor(1, 0), Actor(1, 1), Actor(1, 2)}}});
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_all_adjacent_ally) {
  verify(3, 1, "move_all_adjacent_ally", 
         {::Action::moveAdjacentAlly(0)}, 
         {{::Action::moveAdjacentAlly(0), {Actor(0, 0), Actor(0, 2)}}});
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_all_allies) {
  verify(3, 1, "move_all_allies", 
         {::Action::moveAllAllies(0)}, 
         {{::Action::moveAllAllies(0), {Actor(0, 0), Actor(0, 1), Actor(0, 2), Actor(0, 3), Actor(0, 4), Actor(0, 5)}}});
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_all_enemies) {
  verify(3, 1, "move_all_enemies", 
         {::Action::moveAllEnemies(0)}, 
         {{::Action::moveAllEnemies(0), {Actor(1, 0), Actor(1, 1), Actor(1, 2), Actor(1, 3), Actor(1, 4), Actor(1, 5)}}});
}


TEST_F(EnvironmentVolatileTest, AllTypes3v3_move_side_all) {
  verify(
      3,
      1,
      "move_side_all",
      {::Action::moveSideAll(0)},
      {{::Action::moveSideAll(0), {Actor(1, 1)}}});
}


// TODO: swap actions