/* 
 * File:   trueskill_team.h
 * Author: drendleman
 *
 * Created on September 21, 2020, 4:50 PM
 */

#ifndef TRUESKILL_TEAM_H
#define TRUESKILL_TEAM_H

#include <memory>
#include <vector>

#include "ranked.h"
#include "ranked_team.h"
#include "ranked_evaluator.h"
#include "ranked_planner.h"

struct GroupContribution {
  TrueSkill& skill;
  double contribution;
};

class Battlegroup : public Ranked {
public:

  std::vector<GroupContribution> contributions();

  TrueSkill skill() const;
  RankedTeamPtr team() const { return team_; }
  RankedPlannerPtr planner() const { return evaluator_; }
  RankedEvaluatorPtr evaluator() const { return evaluator_; }
  Hash hash() const { return hash_; }
  
protected:
  GroupContribution synergyContribution() { return GroupContribution{record().skill, 0.1}; }

  RankedTeamPtr team_;
  double teamContribution_;

  RankedEvaluatorPtr evaluator_;
  double evaluatorContribution_;

  RankedPlannerPtr planner_;
  double plannerContribution_;
};

using BattlegroupPtr = std::shared_ptr<Battlegroup>;
using BattlegroupLeague = std::vector<BattlegroupPtr>;

#endif /* TRUESKILL_TEAM_H */
