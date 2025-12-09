add_library(pkaiEngine STATIC
    ../src/ability.cpp
    ../src/action.cpp
    ../src/backpropNet.cpp
    ../src/environment_nonvolatile.cpp
    ../src/environment_possible.cpp
    ../src/environment_volatile.cpp
    ../src/evaluator.cpp
    ../src/evaluator_featureVector.cpp
    ../src/evaluator_montecarlo.cpp
    ../src/evaluator_network128.cpp
    ../src/evaluator_network16.cpp
    ../src/evaluator_network32.cpp
    ../src/evaluator_network64.cpp
    ../src/evaluator_random.cpp
    ../src/evaluator_simple.cpp
    ../src/evaluators.cpp
    ../src/experienceNet.cpp
    ../src/fitness.cpp
    ../src/fixedpoint/fixed_func.cpp
    ../src/game.cpp
    ../src/game_factory.cpp
    ../src/gen4_scripts.cpp
    ../src/init_toolbox.cpp
    ../src/item.cpp
    ../src/league.cpp
    ../src/move.cpp
    ../src/move_nonvolatile.cpp
    ../src/move_volatile.cpp
    ../src/name.cpp
    ../src/nature.cpp
    ../src/neuralNet.cpp
    ../src/order_heuristic.cpp
    ../src/orphan.cpp
    ../src/pkCU.cpp
    ../src/planner.cpp
    ../src/planner_human.cpp
    ../src/planner_max.cpp
    ../src/planner_maximin.cpp
    ../src/planner_minimax.cpp
    ../src/planner_random.cpp
    ../src/planners.cpp
    ../src/pluggable.cpp
    ../src/pokedex.cpp
    ../src/pokedex_dynamic.cpp
    ../src/pokemon_base.cpp
    ../src/pokemon_nonvolatile.cpp
    ../src/pokemon_volatile.cpp
    ../src/ranked.cpp
    ../src/ranked_battlegroup.cpp
    ../src/ranked_evaluator.cpp
    ../src/ranked_planner.cpp
    ../src/ranked_pokemon.cpp
    ../src/ranked_team.cpp
    ../src/ranker.cpp
    ../src/serializable.cpp
    ../src/signature.cpp
    ../src/team_factory.cpp
    ../src/team_nonvolatile.cpp
    ../src/team_volatile.cpp
    ../src/temporalpropNet.cpp
    ../src/trainer.cpp
    ../src/transposition_table.cpp
    ../src/true_skill.cpp
    ../src/type.cpp
)

set_target_properties(pkaiEngine PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(pkaiEngine PUBLIC
    PKAI_STATIC
    FORCE_INLINE=
)

target_link_libraries(pkaiEngine PUBLIC OpenMP::OpenMP_CXX)
