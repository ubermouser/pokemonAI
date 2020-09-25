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

#include "true_skill.h"
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
  struct Contribution {
    double synergy = 1.0;
    double team = 1.0;
    double pokemon = 1.0;
    double planner = 1.0;
    double evaluator = 1.0;

    Contribution(){};
  };

  std::vector<GroupContribution> contributions();
  TrueSkill& computeSkill();

  const RankedTeam& team() const { return *team_; }
  const RankedPlanner& planner() const { return *planner_; }
  const RankedEvaluator& evaluator() const { return *evaluator_; }
  RankedTeam& team() { return *team_; }
  RankedPlanner& planner() { return *planner_; }
  RankedEvaluator& evaluator() { return *evaluator_; }

  virtual void update(const HeatResult& hResult, size_t iTeam) override;

  const std::string& getName() const override { return name_; }

  Battlegroup(
      const RankedTeamPtr& team,
      const RankedEvaluatorPtr& evaluator,
      const RankedPlannerPtr& planner,
      const Contribution& contribution = Contribution{});
protected:
  TrueSkill synergy_;

  RankedTeamPtr team_;

  RankedEvaluatorPtr evaluator_;

  RankedPlannerPtr planner_;

  Contribution contribution_;

  std::string name_;

  virtual Hash generateHash(bool generateSubHashes = true) override;
  virtual std::string defineName() override;
};

using BattlegroupPtr = std::shared_ptr<Battlegroup>;
using BattlegroupLeague = std::vector<BattlegroupPtr>;

#endif /* TRUESKILL_TEAM_H */
