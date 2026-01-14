# Agents

### Code Organization
- Header files are located in the include/pokemonai directory and should be included with the pokemonai/ prefix.

### Building and Testing
- The project is a C++ application built with CMake and managed by Conan.
- The project uses the Google Test (GTest) framework, provided via Conan.
- All executables must be run in the root directory. They rely upon, and pull in assets stored in the `data` directory.
- To install dependencies: `conan install . --output-folder=build --build=missing -s build_type=Release`.
  - **Note**: `libtorch` is not managed by Conan in this configuration. Install it via your system package manager or `pip install torch`.
- To configure:
  - If using system libtorch: `cmake --preset conan-release`.
  - If using pip-installed libtorch: `cmake --preset conan-release -DCMAKE_PREFIX_PATH=$(python3 -c "import torch; print(torch.utils.cmake_prefix_path)")`.
- To build: `cmake --build --preset conan-release -j16 -t pokemonAI -t symlink_scripts`.
- To test: `pushd build/build/Release && ctest --output-on-failure; popd`.
- Individual tests may be run as follows: `./build/build/Release/src/tests/test_name`.