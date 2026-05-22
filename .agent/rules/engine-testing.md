---
trigger: model_decision
description: Writing pokemon engine tests
---

# Engine Move Testing Patterns

When writing or updating move tests for the game engine (e.g., Gen 4 Engine tests), follow these patterns to keep the tests simple, clean, and compartmentalized.

### 1. Test Fixture Setup
- Inherit from the generation-specific test fixture (e.g., `Gen4EngineTest`).
- If the test does not depend on generation-specific logic, prefer the generation-agnostic test fixture `MockEngineTest`.
- Build the non-volatile teams (`TeamNonVolatile`) and environment in the `SetUp()` method.
- Register the environment using `engine_->setEnvironment(env_nv)`.

### 2. Sequential Turn Setup Helpers
- Rather than performing multi-turn engine updates inside individual tests, define small, descriptive helper functions in the test fixture class.
- Chain these helpers sequentially so that each turn updates the state from the previous turn:
  ```cpp
  PossibleEnvironments setupTauntApplied() {
    return engine_->updateState(
        engine_->initialState(), Action::move(0), Action::wait());
  }

  PossibleEnvironments setupTauntTurn2() {
    return engine_->updateState(
        setupTauntApplied().where1(), Action::move(1), Action::move(1));
  }
  ```
- **Important**: Pass valid action choices (e.g., physical moves like Strength rather than status moves like Toxic if the Pokemon is Taunted) to prevent the engine from throwing an validation exception before the turn starts, unless explicitly testing validation failures.

### 3. Idiomatic Assertions Using `where1`
- Use `where` and `where1` helpers to retreive states provided certain criteria.
  ```cpp
  // the highest probability state where at least one of TEAM_A's pokemon hit the target.
  auto state = result.where1Hit(TEAM_A);

  // all states where all pokemon on TEAM_B missed.
  auto states = result.whereMiss(TEAM_B);
  ```
- Instead of using manual `for` loops to scan through possible environments, use the `where1()` API with a lambda predicate to query specific branched states.
  ```cpp
  auto taunted_state = results.where1([](const ConstEnvironmentPossible& res) {
    return res.teammate(TEAM_B, 0).status().cTeammate.taunt_duration != 0;
  });
  ```

### 4. Custom Move and Test Logic
 - Do not modify values within the EnvironmentVolatileData structure directly. Instead, generate the needed state by constructing a sequence of turn setup helpers.
 - If the test requires custom logic, implement this as a non-standard scripted action within `mock_pokedex.hpp`. Add a pokemon that uses the non-standard action to the team, and invoke the move using updateState.