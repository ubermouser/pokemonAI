add_executable(ranker ../src/ranker_main.cpp)

find_package(Boost REQUIRED COMPONENTS program_options filesystem)
find_package(OpenMP REQUIRED)

target_link_libraries(ranker
  pkaiEngine
  Boost::program_options
  Boost::filesystem
  OpenMP::OpenMP_CXX)
