# Pokémon AI

This project is a C++-based Pokémon battle engine and AI. It allows users to simulate Pokémon battles, build teams of pokemon using evolutionary methods, and rank Pokémon teams.

## Project Structure

The project is organized into the following modules:

*   **pkaiEngine**: The core of the project, containing the logic for the Pokémon battle simulator and the AI engine.
*   **battler**: An executable that runs Pokémon battles between two teams, using the `pkaiEngine`.
*   **trainer**: An executable used to build teams of pokemon using evolutionary methods.
*   **ranker**: An executable for ranking Pokémon teams.
*   **gen4_scripts**: Contains scripts and data specific to Generation 4 Pokémon games.
*   **data**: Contains general game data used by the simulator.
*   **teams**: A directory to store Pokémon team files.
*   **networks**: A directory for storing trained neural network models.
*   **src/tests**: Contains the unit tests for the project.

## Building and Running

### Prerequisites

*   CMake 3.23+
*   A C++17 compliant compiler
*   [Conan 2.0+](https://conan.io/) package manager
*   OpenMP (system library)

### Building
#### Using Conan

1.  **Detect Conan Profile:**
    If you haven't already, detect your system's compiler profile:
    ```bash
    conan profile detect --force
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
    cmake --build --preset conan-release -j16 -t pokemonAI -t symlink_scripts
    ```

This will create the executables in the `build/build/Release/` directory (or similar, depending on your environment).

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


#### Trainer

```bash
${BUILD_DIR}/trainer/trainer [options]
```

Construct new teams that play well at a range of skill levels:
```bash
${BUILD_DIR}/trainer/trainer \
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
${BUILD_DIR}/trainer/trainer \
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

#### Tests

To run the tests, use `ctest` from the build directory:

```bash
cd ${BUILD_DIR} && ctest
```
