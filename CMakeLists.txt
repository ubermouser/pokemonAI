cmake_minimum_required(VERSION 3.10)
project(pokemonAI)

enable_testing()
include(CTest)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS program_options filesystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(. include ${Boost_INCLUDE_DIRS})

# Add preprocessor definitions
add_compile_definitions(
    _SCL_SECURE_NO_WARNINGS
    _DISABLEFINEGRAINEDLOCKING
    _DISABLETEMPORALTRACE
    _CONSOLE
    _HTCOLLECTSTATISTICS
    DOUBLEPRECISION
)

# Configuration-specific settings
add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
    $<$<CONFIG:Release>:NDEBUG>
)

# Find Dependencies
find_package(OpenMP REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

# Add subdirectories
add_subdirectory(pkaiEngine)
add_subdirectory(scripts/gen1)
add_subdirectory(scripts/gen4)
add_subdirectory(ranker)
add_subdirectory(trainer)
add_subdirectory(battler)
add_subdirectory(src/tests)

# Add the executable
add_executable(pokemonAI src/battler_main.cpp)

# Link libraries
target_link_libraries(pokemonAI
    pkaiEngine
    gen1_scripts
    gen4_scripts
    Boost::program_options
    Boost::filesystem
    OpenMP::OpenMP_CXX
    fmt::fmt
    spdlog::spdlog
)
