add_library(gen4_scripts SHARED ../src/gen4_scripts.cpp)

target_link_libraries(gen4_scripts pkaiEngine)

if(WIN32)
    target_compile_definitions(gen4_scripts PUBLIC GEN4_SCRIPTS_EXPORTS)
endif()
