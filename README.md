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

*   CMake 3.10+
*   A C++17 compliant compiler
*   Boost (program_options, filesystem)
*   OpenMP

### Building

1.  Create a build directory:
    ```bash
    mkdir build && cd build
    ```
2.  Run CMake:
    ```bash
    cmake ..
    ```
3.  Compile the project:
    ```bash
    make
    ```

This will create the executables in the `build/` directory.

### Running the applications

All executables must be run from the project root directory.

#### Battler

```bash
./build/battler/battler \
    --planner-a=minimax \
    --evaluator-a=simple \
    --team-a=./teams/hexTeamA.txt \
    --team-b=./teams/hexTeamB.txt \
    --num-threads=32 \
    --max-matches=64 \
    --game-verbosity=5
```

#### Trainer

```bash
./build/trainer/trainer
    --planners random max maximin minimax \
    --trainer-verbosity=1 \
    --p1-max-search-depth=0 \
    --p3-max-search-depth=2 \
    --p4-max-search-depth=3 \
    --num-threads=8 \
    --team-path ./teams/
```

#### Ranker

```bash
./build/ranker/ranker \
    --planners random max maximin minimax \
    --ranker-verbosity=1 \
    --p1-max-search-depth=0 \
    --p3-max-search-depth=2 \
    --p4-max-search-depth=3 \
    --num-threads=8 \
    --team-path ./teams/
```

#### Tests

To run the tests, use `ctest` from the build directory:

```bash
cd build && ctest
```
