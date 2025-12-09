#include "pokemonai/ranked_battlegroup.h"

#include <algorithm>
#include <cmath>
#include <boost/functional/hash.hpp>
#include <boost/format.hpp>
#include <vector>


Battlegroup::Battlegroup(
    const RankedTeamPtr& team,
    const RankedEvaluatorPtr& evaluator,
    const RankedPlannerPtr& planner,
    const Contribution& contribution)
  : Ranked(),
    synergy_(TrueSkill::synergy()),
    team_(team),
    evaluator_(evaluator),
    planner_(planner),
    contribution_(contribution) {
  computeSkill();
  identify();
}


std::vector<GroupContribution> Battlegroup::contributions() {
  std::vector<GroupContribution> result = team().contributions();

  result.push_back({synergy_, contribution_.synergy, hash()}); // battlegroup synergy
  result.push_back({planner().skill(), contribution_.planner, planner().hash()}); // planner
  result.push_back({evaluator().skill(), contribution_.evaluator, evaluator().hash()}); // evaluator

  return result;
}


TrueSkill& Battlegroup::computeSkill() {
  team().computeSkill();
  skill() = TrueSkill::combine(contributions());
  return skill();
}


void Battlegroup::update(const HeatResult& hResult, size_t iTeam) {
  Ranked::update(hResult, iTeam);
  team().update(hResult, iTeam);
  planner().update(hResult, iTeam);
  evaluator().update(hResult, iTeam);
}


Battlegroup::Hash Battlegroup::generateHash(bool generateSubHashes) {
  hash_ = 0;
  boost::hash_combine(hash_, team_->hash());
  boost::hash_combine(hash_, evaluator_->hash());
  boost::hash_combine(hash_, planner_->hash());

  return hash_;
}


std::string Battlegroup::defineName() {
  name_ = (boost::format("%s-%s-%s") % planner_->getName() % evaluator_->getName() % team_->getName()).str();
  return name_;
}


std::vector<BattlegroupPtr> BattlegroupLeague::getAll() const {
  //TODO(@drendleman) dedupe with TeamLeague::getAll
  std::vector<BattlegroupPtr> result; result.reserve(size());
  for (auto& battlegroup: *this) { result.push_back(battlegroup.second); }

  return result;
}
