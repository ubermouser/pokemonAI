find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")

foreach(test_file IN LISTS TEST_SOURCES)
    get_filename_component(test_name ${test_file} NAME_WE)

    add_executable(${test_name} ${test_file})

    target_link_libraries(${test_name} pkaiEngine gen4_scripts Boost::program_options Boost::filesystem GTest::GTest GTest::Main)

    gtest_discover_tests(${test_name} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach()
