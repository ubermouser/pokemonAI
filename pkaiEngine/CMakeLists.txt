# option(USE_LEGACY_ENGINE ...) is now in global CMakeLists.txt

set(ENGINE_SOURCES "")
if(USE_LEGACY_ENGINE)
    list(APPEND ENGINE_SOURCES ../src/legacy_pkCU.cpp)
else()
    list(APPEND ENGINE_SOURCES ../src/neo_pkCU.cpp ../src/neo_pkCU_engine.cpp ../src/neo_pkCU_engine_stages.cpp)
endif()

add_library(pkaiEngine SHARED
    ../src/ability.cpp
    ../src/action.cpp
    ../src/actor.cpp
    ../src/environment_nonvolatile.cpp
    ../src/environment_possible.cpp
    ../src/environment_volatile.cpp
    ../src/fixedpoint/fixed_func.cpp
    ../src/init_toolbox.cpp
    ../src/item.cpp
    ../src/logging.cpp
    ../src/move.cpp
    ../src/move_nonvolatile.cpp
    ../src/move_volatile.cpp
    ../src/name.cpp
    ../src/nature.cpp
    ../src/orphan.cpp
    ${ENGINE_SOURCES}
    ../src/pluggable.cpp
    ../src/pokedex.cpp
    ../src/pokedex_dynamic.cpp
    ../src/pokemon_base.cpp
    ../src/pokemon_nonvolatile.cpp
    ../src/pokemon_volatile.cpp
    ../src/serializable.cpp
    ../src/signature.cpp
    ../src/team_factory.cpp
    ../src/team_nonvolatile.cpp
    ../src/team_volatile.cpp
    ../src/type.cpp
    ../src/state_transition_printer.cpp
)

set_target_properties(pkaiEngine PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(pkaiEngine PUBLIC
    FORCE_INLINE=
    USE_LEGACY_ENGINE=$<BOOL:${USE_LEGACY_ENGINE}>
)

target_link_libraries(pkaiEngine
    PUBLIC
    OpenMP::OpenMP_CXX
    fmt::fmt
    spdlog::spdlog
    Boost::filesystem
    Boost::program_options
)
