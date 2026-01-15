add_executable(teambuilder ../src/teambuilder_main.cpp)

find_package(Boost REQUIRED COMPONENTS program_options filesystem)
find_package(OpenMP REQUIRED)

target_link_libraries(teambuilder
  pkaiEngine
  pkaiDecider
  Boost::program_options
  Boost::filesystem
  OpenMP::OpenMP_CXX
  fmt::fmt
  spdlog::spdlog
)
