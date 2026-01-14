add_library(pkaiDecider STATIC
    ../src/neuralNet.cpp
    ../src/evaluator_featureVector.cpp
    ../src/evaluator_network16.cpp
    ../src/evaluator_network32.cpp
    ../src/evaluator_network64.cpp
    ../src/evaluator_network128.cpp
)

target_include_directories(pkaiDecider PUBLIC ../include)

find_library(NUMA_LIB numa)

if(NOT TORCH_LIBRARIES)
    set(TORCH_LIBRARIES libtorch::libtorch)
endif()

target_link_libraries(pkaiDecider
    PUBLIC
    pkaiEngine
    "${TORCH_LIBRARIES}"
    ${NUMA_LIB}
    Boost::program_options
    Boost::filesystem
    fmt::fmt
    spdlog::spdlog
)
