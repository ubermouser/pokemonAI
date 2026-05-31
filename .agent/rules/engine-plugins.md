---
trigger: model_decision
description: Adding a new pokemon move, item, ability, or engine plugin
---

# Engine Plugin Patterns

When implementing new moves, items, or abilities in the game engine (e.g. Gen 4 or Gen 1 scripts), follow these patterns to ensure proper registration, logic deduplication, and targeting correctness.

### 1. Declaring Pointer Variables and Registration Functions
For any new move, item, or ability:
- In `include/pokemonai/genX_scripts_internal.h`:
  - Declare the pointer as an `extern const` variable:
    ```cpp
    extern const item* sitrusBerry_t;
    ```
  - Declare the registration function:
    ```cpp
    void register_item_sitrus_berry(const Pokedex& pkAI, std::vector<plugin>& extensions);
    ```
- Ensure that these are added in sorted alphabetical order.

### 2. Defining and Registering Pointers
- In `src/genX/genX_scripts_internal.cpp`:
  - Define the pointer at the top of the file:
    ```cpp
    const item* sitrusBerry_t = nullptr;
    ```
  - In the `registerGenXExtensions` function, resolve and initialize the pointer:
    ```cpp
    orphan::orphanCheck(sitrusBerry_t, pkAI.item("sitrus berry"));
    ```
  - Call the registration helper to register the plugin extensions:
    ```cpp
    register_item_sitrus_berry(pkAI, extensions);
    ```

### 3. Implementation File Structure
Create a dedicated source file in `src/genX/` (e.g., `src/gen4/item_sitrus_berry.cpp`):
- If the code would benefit from logic reuse, extract that logic to a common helper function defined in an anonymous namespace.

### 4. Correct Event Hooks and Targets
- Refer to `pluggable.h` and `pkCU_types.h` for a description of what plugin types exist and when they trigger. 
- Plugins may trigger on either the current_team, the other_team, or all_teams. Use all_teams plugins sparingly as they can trigger many times per state transition.

### 5. Testing your extension
- Every engine extension must be tested heavily to prevent regressions. Refer to `engine-testing.md` for best patterns on how to test your new engine extension.

### 6. Manual Verification
- Use the newly developed engine effect in a real game and verify that the effect behaves as expected. Do this with the battler CLI - see `interactive-debugging.md` for how to do this.