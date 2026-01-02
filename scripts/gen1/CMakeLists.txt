add_library(gen1_scripts SHARED ../../src/gen1_scripts.cpp)

target_link_libraries(gen1_scripts
    pkaiEngine
)
