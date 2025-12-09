cmake_minimum_required(VERSION 3.10)
project(pokemonAI)

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
    WIN32
    _SCL_SECURE_NO_WARNINGS
    _DISABLEFINEGRAINEDLOCKING
    _DISABLETEMPORALTRACE
    _CONSOLE
    _HTCOLLECTSTATISTICS
    DOUBLEPRECISION
    GEN4_STATIC
)

# Configuration-specific settings
add_compile_definitions(
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Add subdirectories
add_subdirectory(pkaiEngine)
add_subdirectory(gen4_scripts)
add_subdirectory(ranker)
add_subdirectory(trainer)
add_subdirectory(battler)

# Add the executable
add_executable(pokemonAI src/battler_main.cpp)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Link libraries
target_link_libraries(pokemonAI pkaiEngine gen4_scripts Boost::program_options Boost::filesystem OpenMP::OpenMP_CXX)
