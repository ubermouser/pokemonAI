# Agents

### Code Organization
- Header files are located in the include/pokemonai directory and should be included with the pokemonai/ prefix.

### Building and Testing
- The project is a C++ application built with CMake and managed by Conan.
- The project uses the Google Test (GTest) framework, provided via Conan.
- All executables must be run in the root directory. They rely upon, and pull in assets stored in the `data` directory.
- To install dependencies (Debug with Sanitizers): `conan install . --output-folder=build --build=missing -s build_type=Release -s "&:build_type=Debug"`.
- To configure: `cmake --preset conan-debug`.
- To build: `cmake --build --preset conan-debug`.
- To test: `ctest --preset conan-debug`.
- Individual tests: `./build/build/Release/src/tests/test_name`.