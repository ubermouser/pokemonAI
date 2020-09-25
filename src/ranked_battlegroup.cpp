#include "../inc/ranked_battlegroup.h"

#include <boost/functional/hash.hpp>
#include <boost/format.hpp>


Battlegroup::Battlegroup(
    const RankedTeamPtr& team,
    const RankedEvaluatorPtr& evaluator,
    const RankedPlannerPtr& planner,
    const Contribution& contribution)
  : team_(team),
    evaluator_(evaluator),
    planner_(planner),
    contribution_(contribution) { 
  identify();
}


std::vector<GroupContribution> Battlegroup::contributions() {
  std::vector<GroupContribution> result;

  result.push_back({skill(), contribution_.synergy}); // battlegroup synergy
  result.push_back({planner().skill(), contribution_.planner}); // planner
  result.push_back({evaluator().skill(), contribution_.evaluator}); // evaluator
  result.push_back({team().skill(), contribution_.team}); // team synergy

  // individual pokemon
  for (RankedPokemonPtr& pokemon: team().teammates()) {
    result.push_back({pokemon->skill(), contribution_.pokemon});
  }
  return result;
}


void Battlegroup::update(const HeatResult& hResult, size_t iTeam) {
  Ranked::update(hResult, iTeam);
  team().update(hResult, iTeam);
  planner().update(hResult, iTeam);
  evaluator().update(hResult, iTeam);
}


Battlegroup::Hash Battlegroup::generateHash(bool generateSubHashes) {
  hash_ = 0;
  boost::hash_detail::hash_combine_impl(hash_, team_->hash());
  boost::hash_detail::hash_combine_impl(hash_, evaluator_->hash());
  boost::hash_detail::hash_combine_impl(hash_, planner_->hash());

  return hash_;
}


std::string Battlegroup::defineName() {
  name_ = (boost::format("%s %s") % planner_->getName() % team_->getName()).str();
  return name_;
}
