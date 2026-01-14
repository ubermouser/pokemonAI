# Agents

### Code Organization
- Header files are located in the include/pokemonai directory and should be included with the pokemonai/ prefix.

### Building and Testing
- The project is a C++ application built with CMake and managed by Conan.
- The project uses the Google Test (GTest) framework, provided via Conan.
- All executables must be run in the root directory. They rely upon, and pull in assets stored in the `data` directory.
- To install dependencies: `conan install . --output-folder=build --build=missing -s build_type=Release`.
- To configure: `cmake --preset conan-release`.
- To build: `cmake --build --preset conan-release -j16`.
- To test: `pushd build/build/Release && ctest --output-on-failure; popd`.
- Individual tests may be run as follows: `./build/build/Release/src/tests/test_name`.