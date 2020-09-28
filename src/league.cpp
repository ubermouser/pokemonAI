#include "../inc/league.h"


League& League::addTeam(const RankedTeamPtr& team) {
  if (teams.insert({team->hash(), team}).second) {
    addTeamBattlegroups(team);
  }
  
  return *this;
}


League& League::addPlanner(const RankedPlannerPtr& planner) {
  if (planners.insert({planner->hash(), planner}).second) {
    addPlannerBattlegroups(planner);
  }
  return *this;
}


League& League::addEvaluator(const RankedEvaluatorPtr& evaluator) {
  if (evaluators.insert({evaluator->hash(), evaluator}).second) {
    addEvaluatorBattlegroups(evaluator);
  }
  return *this;
}


template<class Hash, class AdjacencyMapType, class LeagueType>
size_t League::removeEntity(
    const Hash& entityHash, AdjacencyMapType& adjacencyMap, LeagueType& league) {
  size_t numErased = 0;
  // construct a copy of the set, as removal invalidates the iterator:
  auto adjacencySet = adjacencyMap.at(entityHash);
  for (const auto& bgHash : adjacencySet) {
    numErased += removeBattlegroup(bgHash);
  }
  league.erase(entityHash);
  adjacencyMap.erase(entityHash);
  return numErased;
}


size_t League::removeTeam(const RankedTeam::Hash& teamHash) {
  return removeEntity(teamHash, team_to_battlegroup_, teams);
}


size_t League::removePlanner(const RankedPlanner::Hash& plannerHash) {
  return removeEntity(plannerHash, planner_to_battlegroup_, planners);
}


size_t League::removeEvaluator(const RankedEvaluator::Hash& evaluatorHash) {
  return removeEntity(evaluatorHash, evaluator_to_battlegroup_, evaluators);
}


size_t League::addEvaluatorBattlegroups(const RankedEvaluatorPtr& evaluator) {
  size_t numAdded = teams.size() * planners.size();
  battlegroups.reserve(battlegroups.size() + numAdded);
  for (auto& team: teams) {
    for (auto& planner: planners) {
      addBattlegroup(team.second, planner.second, evaluator);
    }
  }
  return numAdded;
}


size_t League::addTeamBattlegroups(const RankedTeamPtr& team) {
  size_t numAdded = planners.size() * evaluators.size();
  battlegroups.reserve(battlegroups.size() + numAdded);
  for (auto& planner: planners) {
    for (auto& evaluator: evaluators) {
      addBattlegroup(team, planner.second, evaluator.second);
    }
  }
  return numAdded;
}


size_t League::addPlannerBattlegroups(const RankedPlannerPtr& planner) {
  size_t numAdded = teams.size() * evaluators.size();
  battlegroups.reserve(battlegroups.size() + numAdded);
  for (auto& team: teams) {
    for (auto& evaluator: evaluators) {
      addBattlegroup(team.second, planner, evaluator.second);
    }
  }
  return numAdded;
}


void League::addBattlegroup(
    const RankedTeamPtr& team,
    const RankedPlannerPtr& planner,
    const RankedEvaluatorPtr& evaluator) {
  BattlegroupPtr battlegroup =
      std::make_shared<Battlegroup>(team, evaluator, planner);
  if (battlegroups.insert({battlegroup->hash(), battlegroup}).second) {
    // if an insertion successfully took place, add the battlegroup to adjacency maps:
    team_to_battlegroup_[team->hash()].insert(battlegroup->hash());
    evaluator_to_battlegroup_[evaluator->hash()].insert(battlegroup->hash());
    planner_to_battlegroup_[planner->hash()].insert(battlegroup->hash());
  }
}


size_t League::removeBattlegroup(const Battlegroup::Hash& id) {
  team_to_battlegroup_.at(battlegroups.at(id)->team().hash()).erase(id);
  evaluator_to_battlegroup_.at(battlegroups.at(id)->evaluator().hash()).erase(id);
  planner_to_battlegroup_.at(battlegroups.at(id)->planner().hash()).erase(id);
  return battlegroups.erase(id);
}


size_t League::removeUnusedPokemon() {
  size_t previousSize = pokemon.size();
  pokemon.clear();
  for (auto& team : teams) {
    for (auto& pk : team.second->teammates()) {
      pokemon.insert({pk->hash(), pk});
    }
  }

  return previousSize - pokemon.size();
}


/*size_t League::recomputeAdjacentRanks(const BattlegroupPtr& battlegroup) {
  for (auto& bgHash: team_to_battlegroup_.at(battlegroup->team().hash())) {
    battlegroups.at(bgHash)->computeSkill();
  }
  for (auto& bgHash: planner_to_battlegroup_.at(battlegroup->planner().hash())) {
    battlegroups.at(bgHash)->computeSkill();
  }
  for (auto& bgHash: evaluator_to_battlegroup_.at(battlegroup->evaluator().hash())) {
    battlegroups.at(bgHash)->computeSkill();
  }
}*/