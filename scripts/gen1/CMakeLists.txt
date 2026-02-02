file(GLOB_RECURSE GEN1_SCRIPTS_SOURCES "../../src/gen1/*.cpp")

add_library(gen1_scripts SHARED ${GEN1_SCRIPTS_SOURCES})

target_include_directories(gen1_scripts PRIVATE ../../src/gen1)

target_link_libraries(gen1_scripts
    pkaiEngine
)
