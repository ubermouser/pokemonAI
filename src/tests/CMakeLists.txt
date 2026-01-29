find_package(GTest REQUIRED)
include(GoogleTest)

if(NOT TORCH_LIBRARIES)
    set(TORCH_LIBRARIES libtorch::libtorch)
endif()

find_package(OpenBLAS REQUIRED)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")

set(DECIDER_TESTS
    test_evaluator
    test_planner
    test_planner_human
    test_network_evaluator
    test_trainer
    test_trainer_regress_fitness
    test_teambuilder
    test_game
    test_fitness
    test_ranker
    test_ml_framework
    test_torch_minimal
    test_blas_minimal
)

foreach(test_file IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name} ${test_file})

    set(TEST_LIBS
        pkaiEngine
        Boost::program_options
        Boost::filesystem
        fmt::fmt
        spdlog::spdlog
        GTest::gtest
        GTest::gtest_main
    )

    if(test_name IN_LIST DECIDER_TESTS)
        list(APPEND TEST_LIBS 
            pkaiDecider
            "${TORCH_LIBRARIES}"
            OpenBLAS::OpenBLAS)
    endif()

    target_link_libraries(${test_name} ${TEST_LIBS})

    gtest_discover_tests(${test_name} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach()
