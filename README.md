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
*   fmt 12.1+ ( [String formatting](https://fmt.dev/) )
*   spdlog 1.16+ ( [Fast logging framework](https://github.com/gabime/spdlog) )

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
./build/battler/battler [options]
```

Battle two teams against one another using minimax search:
```bash
./build/battler/battler \
    --team-a ./teams/hexTeamD.txt \
    --team-b ./teams/hexTeamA.txt \
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
./build/battler/battler \
    --team-a ./teams/hexTeamD.txt \
    --team-b ./teams/hexTeamA.txt \
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
./build/trainer/trainer [options]
```

Construct new teams that play well at a range of skill levels:
```bash
./build/trainer/trainer \
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
./build/trainer/trainer \
    --planners=random max minimax minimax \
    --evaluators=simple \
    --p1-max-search-depth=0 \
    --p2-max-search-depth=1 \
    --p3-max-search-depth=2 \
    --p4-max-search-depth=3 \
    --ranker-verbosity=1 \
    --verbosity=2 \
    --prefix-path data/gen1 \
    --plugins plugins/gen1 \
    --num-threads=32
```

#### Ranker

```bash
./build/ranker/ranker [options]
```


Rank all of the teams under a directory against one another:
```bash
./build/ranker/ranker \
    --team-path ./teams/ \
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
cd build && ctest
```
