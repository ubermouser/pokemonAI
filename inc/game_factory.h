/* 
 * File:   game_factory.h
 * Author: drendleman
 *
 * Created on September 21, 2020, 8:59 PM
 */

#ifndef GAME_FACTORY_H
#define GAME_FACTORY_H

#include "true_skill.h"
#include "ranked_battlegroup.h"

class GameFactory {
public:
  struct Config {
    /* speed of rank propagation */
    double dynamicsFactor = TrueSkill::initialMean() / 300.;

    /* probability of a draw given two equal teams */
    double drawProbability = 0.1;

    /* proportion of a win/loss/draw that an evaluator is responsible for */
    double evaluatorContribution = 0.5;

    Config(){};
  };

  GameFactory(const Config& cfg = Config{});

  TrueSkill create(size_t numTeammates) const;

  TrueSkill feather(const TrueSkill& source) const;

  /* determine the quality of a match between two teams */
  double matchQuality(const Battlegroup& team_a, const Battlegroup& team_b) const;

  size_t update(Battlegroup& team_a, Battlegroup& team_b, const HeatResult& gResult) const;

protected:
  static double calculateDrawMargin(double drawProbability, double beta);

  /* determine if performance is greater than draw margin */
  static double VExceedsMargin(double dPerformance, double drawMargin);
  static double WExceedsMargin(double dPerformance, double drawMargin);

  /* determine if performance is within margin */
  static double VWithinMargin(double dPerformance, double drawMargin);
  static double WWithinMargin(double dPerformance, double drawMargin);

  void update_ranked(
    TrueSkill& cSkill,
    double v,
    double w,
    double c,
    double cSquared,
    double rankMultiplier,
    double partialPlay) const;

  Config cfg_;

  /* equivalent to performanceStdDev squared */
  double betaSquared_;

  double drawMargin_;

  double tauSquared_;
};

#endif /* GAME_FACTORY_H */

