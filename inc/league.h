/* 
 * File:   league.h
 * Author: drendleman
 *
 * Created on September 28, 2020, 10:42 AM
 */

#ifndef LEAGUE_H
#define LEAGUE_H

#include <unordered_map>
#include <unordered_set>

#include "ranked_battlegroup.h"
#include "ranked_team.h"
#include "ranked_pokemon.h"
#include "ranked_evaluator.h"
#include "ranked_planner.h"

template<typename U, typename T>
using AdjacencyMap = std::unordered_map<U, std::unordered_set<T> >;

struct League {
  PlannerLeague planners;

  EvaluatorLeague evaluators;

  TeamLeague teams;

  PokemonLeague pokemon;

  BattlegroupLeague battlegroups;

  League& addTeam(const RankedTeamPtr& team);
  size_t removeTeam(const RankedTeam::Hash& hash);

  League& addPlanner(const RankedPlannerPtr& planner);
  size_t removePlanner(const RankedPlanner::Hash& hash);

  League& addEvaluator(const RankedEvaluatorPtr& planner);
  size_t removeEvaluator(const RankedEvaluator::Hash& hash);

  size_t removeUnusedPokemon();

  //size_t recomputeAdjacentRanks(const BattlegroupPtr& battlegroup);

protected:
  size_t addEvaluatorBattlegroups(const RankedEvaluatorPtr& evaluator);
  size_t addTeamBattlegroups(const RankedTeamPtr& team);
  size_t addPlannerBattlegroups(const RankedPlannerPtr& planner);
  
  void addBattlegroup(
      const RankedTeamPtr& team,
      const RankedPlannerPtr& planner,
      const RankedEvaluatorPtr& evaluator);
  
  size_t removeBattlegroup(const Battlegroup::Hash& id);
  
  template<class Hash, class AdjacencyMapType, class LeagueType>
  size_t removeEntity(const Hash& entityHash, AdjacencyMapType& adjacencyMap, LeagueType& league);

  AdjacencyMap<RankedTeam::Hash, Battlegroup::Hash> team_to_battlegroup_;

  AdjacencyMap<RankedEvaluator::Hash, Battlegroup::Hash> evaluator_to_battlegroup_;

  AdjacencyMap<RankedPlanner::Hash, Battlegroup::Hash> planner_to_battlegroup_;

  //AdjacencyMap<RankedPokemon::Hash, Battlegroup::Hash> pokemon_to_team_;
};

#endif /* LEAGUE_H */
