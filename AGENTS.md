# Agents

### Code Organization
- Header files are located in the include/pokemonai directory and should be included with the pokemonai/ prefix.

### Building and Testing
- The project is a C++ application built with CMake.
- The project uses the Google Test (GTest) framework for C++ unit testing.
- All executables must be run in the root directory. They rely upon, and pull in assets stored in the `data` directory.
- To build, run `rm -R build/* && pushd build && cmake .. -D CMAKE_BUILD_TYPE=Debug && make -j16; popd`.
- To rebuild without configuring, `pushd build && ctest; popd`.
- To test, run `pushd build && ctest --output-on-failure; popd`
- Individual tests may be run as follows: `./build/src/tests/test_name`.