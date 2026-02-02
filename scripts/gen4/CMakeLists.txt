file(GLOB_RECURSE GEN4_SCRIPTS_SOURCES "../../src/gen4/*.cpp")

add_library(gen4_scripts SHARED ${GEN4_SCRIPTS_SOURCES})

target_include_directories(gen4_scripts PRIVATE ../../src/gen4)

target_link_libraries(gen4_scripts
    pkaiEngine
)
