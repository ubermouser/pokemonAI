add_library(pkaiDecider STATIC
    ../src/app_utils.cpp
    ../src/neuralNet.cpp
    ../src/evaluator_network.cpp
    ../src/evaluator_network16.cpp
    ../src/evaluator_network32.cpp
    ../src/evaluator_network64.cpp
    ../src/evaluator_network128.cpp
    ../src/evaluator_network_large.cpp
    ../src/trainable_neural_net.cpp
    ../src/trainer_regress_fitness.cpp
    ../src/trainer.cpp
    ../src/evaluators.cpp
    ../src/order_heuristic.cpp
    ../src/fitness.cpp
    ../src/game.cpp
    ../src/game_factory.cpp
    ../src/league.cpp
    ../src/league_heat.cpp
    ../src/ranked.cpp
    ../src/ranked_battlegroup.cpp
    ../src/ranked_pokemon.cpp
    ../src/ranked_team.cpp
    ../src/ranker.cpp
    ../src/teambuilder.cpp
    ../src/transposition_table.cpp
    ../src/true_skill.cpp
    ../src/evaluator.cpp
    ../src/evaluator_montecarlo.cpp
    ../src/evaluator_random.cpp
    ../src/evaluator_simple.cpp
    ../src/planner.cpp
    ../src/planner_human.cpp
    ../src/planner_max.cpp
    ../src/planner_maximin.cpp
    ../src/planner_minimax.cpp
    ../src/planner_negamax.cpp
    ../src/planner_softmax.cpp
    ../src/planner_random.cpp
    ../src/planners.cpp
    ../src/ranked_evaluator.cpp
    ../src/ranked_planner.cpp
)

target_include_directories(pkaiDecider PUBLIC ../include)

if(NOT TORCH_LIBRARIES)
    set(TORCH_LIBRARIES libtorch::libtorch)
endif()

target_link_libraries(pkaiDecider
    PUBLIC
    pkaiEngine
    "${TORCH_LIBRARIES}"
    Boost::program_options
    Boost::filesystem
    fmt::fmt
    spdlog::spdlog
)
