---
trigger: model_decision
description: Manual verification of engine behavior using the interactive CLI
---

# Interactive Debugging and Bug Finding with the Battler

This guide describes how to use the interactive `battler` interface to find engine bugs, diagnose command-line quirks, and identify fatal errors in the Pokémon AI simulation.

---

## 1. Running the Battler in Interactive Mode

To play interactive games against an AI planner, execute the `battler` tool from the project root with the human planner option:

```bash
./build/build/Debug/battler/battler --config ./configs/battler_2v2test.cfg --planner-b=human --a-max-search-depth=1
```

### Key Command Line Options
* `--config <path>`: Specifies the team setup and battle configuration (e.g., 2v2 test).
* `--planner-b=human` / `--planner-a=human`: Assigns a human operator (stdin/stdout) to control Team A or Team B.
* `--a-max-search-depth=<depth>`: Configures the maximum depth for the AI's search engine (lower depth speeds up interactive play).
* `--allow-state-selection=1`: Normally, the game will perform random state transitions to mimic the real game. You can force your own state transitions to coerce specific behaviors using this option.

---

## 2. Interacting with the CLI

### Action Selection
When prompting for actions, the console prints the currently active Pokemon and their available options:

```
Active pokemon TB:1: "MAtkLead"-"machamp" 381/381
	"ice punch" [ice] Physical Pow: 75 PP: 24/24
	[m4-1 (-0.12), m4-2 (-0.09), m4-f2 (-0.14)]
```

* **Action Codes**: Format is `m<slot>-<target>`, where:
  * `m4` represents the move slot (1-4).
  * `-1` or `-2` represents hostile targets (`TA:1`, `TA:2`).
  * `-f2` represents friendly targets on your own team.
* **Wait Action**: Represented by `w` (nothing/wait).
* **Switch Action**: Represented by `s<bench_index>` (e.g., `s3`).
* **Activation**: Represented by bringing sidelined pokemon into empty slots (e.g. `tb:3` or `tb:4`).
* **Fitness Deltas**: The values in parentheses (e.g., `(-0.12)`) represent the expected change in evaluator fitness if that action is selected.

### Target Parsing Syntax
To select an actor or target, type it in the format:
`t[a|b]:?[1-6]` (e.g., `tb:3`, `ta1`, `tb4`).

---

## 3. Troubleshooting Checklist for Developers

* **Rebuilding plugins**: If you modify move, ability, or item scripts, they are dynamically linked and won't be rebuilt by just running tests. Make sure to rebuild the target scripts specifically:
  ```bash
  cmake --build --preset conan-debug --target gen4_scripts
  ```
* **Verify Startup Logs**: Check stderr/stdout at start of execution for warnings regarding orphaned moves or unimplemented abilities.
* **Observe the State**: Watch out for incorrect type interactions (e.g., status effects applied to immune Pokemon) or invalid targets.
