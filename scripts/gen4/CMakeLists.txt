add_library(gen4_scripts SHARED ../../src/gen4_scripts.cpp)

target_link_libraries(gen4_scripts
    pkaiEngine
)
