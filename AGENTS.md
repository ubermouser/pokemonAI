---
trigger: always_on
---

# Agents

### Code Organization
- Header files are located in the `include/pokemonai` directory and should be included with the `pokemonai/` prefix.
- Generation-specific code (Gen 1 and Gen 4) is in `src/gen1/` and `src/gen4/`.
- New move/ability implementations should be added as separate files in these directories and registered in the corresponding `genX_scripts_internal.cpp`.

### Building and Testing
- The project is a C++ application built with CMake and managed by Conan.
- The project uses the Google Test (GTest) framework, provided via Conan.
- All executables must be run in the root directory. They rely upon, and pull in assets stored in the `data` directory.
- To install dependencies (Debug with Sanitizers): `conan install . --output-folder=build --build=missing -s build_type=Release -s "&:build_type=Debug"`.
- To configure: `cmake --preset conan-debug`.
- To build: `cmake --build --preset conan-debug`.
- To test: `ctest --preset conan-debug`.
- Individual tests: `./build/build/Debug/src/tests/test_name`.

### Development Patterns and Quirks
- **Test Logic**: Generation specific code is dynamically linked and not recompiled with the test. You must rebuild it separately with `cmake --build --preset conan-debug --target gen4_scripts`.
- For in-depth help writing engine tests, see `.agent/rules/engine-testing.md`
- For in-depth help extending the engine with new actions, see `.agent/rules/engine-plugins.md`.