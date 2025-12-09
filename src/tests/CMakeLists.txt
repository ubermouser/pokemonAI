find_package(GTest REQUIRED)

add_executable(test_game test_game.cpp)
add_executable(test_engine test_engine.cpp)
add_executable(test_evaluator test_evaluator.cpp)

target_link_libraries(test_game pkaiEngine Boost::program_options Boost::filesystem GTest::GTest GTest::Main)
target_link_libraries(test_engine pkaiEngine Boost::program_options Boost::filesystem GTest::GTest GTest::Main)
target_link_libraries(test_evaluator pkaiEngine Boost::program_options Boost::filesystem GTest::GTest GTest::Main)

include(GoogleTest)
gtest_discover_tests(test_game WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
gtest_discover_tests(test_engine WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
gtest_discover_tests(test_evaluator WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
