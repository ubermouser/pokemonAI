# Pokémon AI

This project is a C++-based Pokémon battle engine and AI. It allows users to simulate Pokémon battles, build teams of pokemon using evolutionary methods, and rank Pokémon teams.

## Project Structure

The project is organized into the following modules:

*   **pkaiEngine**: The core of the project, containing the logic for the Pokémon battle simulator.
*   **pkaiDecider**: A library containing the logic for the AI engine, all planners, evaluators, and trainers.
*   **battler**: An executable that runs Pokémon battles between two teams, using `pkaiEngine`.
*   **teambuilder**: An executable used to build teams of pokemon using evolutionary methods.
*   **trainer**: An executable used to training machine-learning based planners and evaluators.
*   **ranker**: An executable for ranking Pokémon planners, evaluators, and teams.
*   **genX_scripts**: Contains scripts and data specific to a specific generation of Pokémon games. Currently, Generations 1 (RB) and 4 (DP) are supported.
*   **data**: Contains general game data used by the simulator.
*   **teams**: A directory to store Pokémon team files.
*   **networks**: A directory for storing trained neural network models.
*   **src/tests**: Contains the unit tests for the project.

## Extending the Engine

To implement new moves or abilities for a specific generation:
1.  **Register Script Pointer**: Abilities / Moves / Pokemon / Items / etc must have an equivalent name in the corresponding data file, e.g. inside `data/genX/moves.csv`.
2.  **Implement Plugin**: Add the implementation to the corresponding `src/genX_scripts.cpp`. Use the appropriate plugin hook for custom logic (e.g. `PLUGIN_ON_EVALUATEMOVE` for Counter/Mirror Coat).
3.  **Maintain Order**: Keep move pointers and registrations in `genX_scripts.cpp` alphabetical.
4.  **Verification**: Add a test in `src/tests/` using `GenXEngineTest` (or equivalent) to verify the new logic.

## Building and Running

### Prerequisites

*   CMake 3.23+
*   A C++17 compliant compiler
*   [Conan 2.0+](https://conan.io/) package manager
*   OpenMP (system library)

### Building
#### Using Conan

##### Release
1.  **Detect Conan Profile:**
    If you haven't already, detect your system's compiler profile:
    ```bash
    conan profile detect --exist-ok
    ```

2.  **Install Dependencies:**
    Use Conan to install the required libraries (Boost, fmt, spdlog, GTest):
    ```bash
    conan install . --output-folder=build --build=missing -s build_type=Release
    ```

3.  **Configure and Build:**
    Use the Conan-generated CMake preset to configure and build the project:
    ```bash
    cmake --preset conan-release
    cmake --build --preset conan-release
    ```

##### Debug (with Sanitizers)
Debug mode enables LLVM sanitizers (Address, Undefined, and Leak) and is recommended for development.

1.  **Install Dependencies:**
    We build the transitive dependencies in Release mode to avoid build failures in `openblas`.
    ```bash
    conan install . --output-folder=build --build=missing -s build_type=Release -s "&:build_type=Debug"
    ```

2.  **Configure and Build:**
    ```bash
    cmake --preset conan-debug
    cmake --build --preset conan-debug
    ```

This will create the executables in the `build/build/Release/` or `build/build/Debug/` directory.

#### Using System Dependencies

1.  Create a build directory:
    ```bash
    mkdir build && cd build
    ```
2.  Run CMake:
    ```bash
    cmake -D CMAKE_BUILD_TYPE=Release ..
    ```
3.  Compile the project:
    ```bash
    make -j32
    ```

Executables will be under the `build` directory.

### Testing

#### System build:

To run the tests, use `ctest` from the build directory:

```bash
pushd build && ctest --output-on-failure; popd
```

Individual tests:
```bash
./build/src/tests/test_name
```

#### Conan:

All tests: 

```bash
ctest --preset conan-release --output-on-failure
# or
ctest --preset conan-debug --output-on-failure
```

Individual test:

```bash
./build/build/Release/src/tests/test_name
# or
./build/build/Debug/src/tests/test_name
```

### Running the applications

All executables must be run from the project root directory.

#### Battler

```bash
${BUILD_DIR}/battler/battler [options]
```

Battle two teams against one another using minimax search:
```bash
${BUILD_DIR}/battler/battler \
    --team-a ./teams/gen4/hexTeamD.txt \
    --team-b ./teams/gen4/hexTeamA.txt \
    --planner-a minimax \
    --planner-b minimax \
    --evaluator-a simple \
    --evaluator-b simple \
    --b-max-search-depth=4 \
    --a-max-search-depth=4 \
    --a-planner-verbosity=2 \
    --b-planner-verbosity=2 \
    --game-verbosity=3 \
    --verbosity=2
```

Play against an agent yourself:
```bash
${BUILD_DIR}/battler/battler \
    --team-a ./teams/gen4/hexTeamD.txt \
    --team-b ./teams/gen4/hexTeamA.txt \
    --planner-a minimax \
    --planner-b human \
    --evaluator-a simple \
    --b-max-search-depth=4 \
    --a-planner-verbosity=2 \
    --b-planner-verbosity=2 \
    --game-verbosity=3 \
    --verbosity=2
```


#### TeamBuilder

```bash
${BUILD_DIR}/teambuilder/teambuilder [options]
```

Construct new teams that play well at a range of skill levels:
```bash
${BUILD_DIR}/teambuilder/teambuilder \
    --planners=random max minimax minimax \
    --evaluators=simple \
    --p1-max-search-depth=0 \
    --p2-max-search-depth=1 \
    --p3-max-search-depth=2 \
    --p4-max-search-depth=3 \
    --ranker-verbosity=1 \
    --verbosity=2 \
    --num-threads=32
```

Construct teams targeting a different generation of Pokemon:
```bash
${BUILD_DIR}/teambuilder/teambuilder \
    --team-path ./teams/gen1/ \
    --planners=minimax \
    --evaluators=simple \
    --p1-max-search-depth=3 \
    --ranker-verbosity=1 \
    --verbosity=2 \
    --prefix-path data/gen1 \
    --plugins plugins/gen1 \
    --num-threads=32
```

#### Trainer

```bash
${BUILD_DIR}/trainer/trainer [options]
```

Train two `TrainableNeuralNet` evaluators through self-play with a set of 
randomly generated teams:
```bash
${BUILD_DIR}/trainer/trainer \
    --planners=softmax softmax \
    --evaluators=network16 network64 simple random \
    --p1-max-search-depth=1 \
    --p2-max-search-depth=1 \
    --p2-temperature=0.25 \
    --e1-net-architecture 64 \
    --e1-net-model-path ./networks/net16-64.model \
    --e2-net-architecture 128 \
    --e2-net-model-path ./networks/net64-128.model \
    --save-on-completion=1 \
    --allow-same-planner=1 \
    --training-epochs=25 \
    --max-generations=50 \
    --ranker-verbosity=2 \
    --verbosity=2 \
    --num-threads=16
```

#### Ranker

```bash
${BUILD_DIR}/ranker/ranker [options]
```


Rank all of the teams under a directory against one another:
```bash
${BUILD_DIR}/ranker/ranker \
    --team-path ./teams/gen4/ \
    --planners=minimax \
    --evaluators=simple \
    --p1-max-search-depth=3 \
    --ranker-verbosity=1 \
    --verbosity=2 \
    --num-threads=16
```
