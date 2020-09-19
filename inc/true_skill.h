#ifndef TRUESKILL_H
#define TRUESKILL_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>
#include <array>
#include <ostream>


struct TrueSkill {
  /* measure of the mean of a given object's skill */
  double mean;

  /* measure of the variance of a given object's skill */
  double stdDev;

  double getRank() const { return mean - (stdDev * 3.0); }
  
  bool operator <(const TrueSkill& other) const { return getRank() < other.getRank(); };
  bool operator >(const TrueSkill& other) const { return getRank() > other.getRank(); };

  void output(std::ostream& oFile, bool printHeader = true) const;
  bool input(const std::vector<std::string>& lines, size_t& firstLine);
};


class TrueSkillFactory {
public:
  struct Config {
    /* mean of prior skill belief */
    double initialMean = 25.0;

    /* standard deviation of prior skill belief */
    double initialStdDev = initialMean / 3.0;

    /* standard deviation of performance; the difference in points necessary to assert
    a team will have an 80% : 20% ratio of winning : losing */
    double performanceStdDev = initialMean / 6.0;

    /* probability of a draw given two equal teams */
    double drawProbability = 0.1;

    /* speed of rank propagation */
    double dynamicsFactor = initialMean / 300.;

    /* proportion of a win/loss/draw that an evaluator is responsible for */
    double evaluatorContribution = 0.5;
  };

  TrueSkill create(size_t numTeammates) const;

  TrueSkill feather(const TrueSkill& source) const;

  /* determine the quality of a match between two teams */
  double matchQuality(const TrueSkillTeam& team_a, const TrueSkillTeam& team_b) const;

  size_t update(const TrueSkillTeam& team_a, const TrueSkillTeam& team_b, const GameResult& gResult) const;

  void teamRank_finalize(TrueSkill& cResult, double currentProportion, double totalProportion);

  /* estimate the rank of a team given its component teams */
  template<class constRankedIterator_t, class constDoubleIterator_t>
  TrueSkill teamRank(
    constRankedIterator_t iComponent,
    constRankedIterator_t componentEnd,
    constDoubleIterator_t iProportion,
    fpType totalProportion = 1.0)
  {
    fpType currentProportion = 0;
    TrueSkill cResult(0, 0);

    for ( ;iComponent != componentEnd; ++iComponent, ++iProportion)
    {
      const TrueSkill& cSkill = (*iComponent)->getSkill();

      cResult.mean += cSkill.mean * (*iProportion / totalProportion);
      cResult.stdDev += (cSkill.stdDev*cSkill.stdDev) * (*iProportion / totalProportion);

      currentProportion += *iProportion;
    }

    teamRank_finalize(cResult, currentProportion, totalProportion);
    return cResult;
  }; // endOf teamRank

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

  /* equivalent to performanceStdDev squared */
  double betaSquared;

  double drawMargin;

  double tauSquared;
};


struct TrueSkillTeam {
  TrueSkill aggregateSkill;
  std::shared_ptr<RankedTeam> team;
  std::shared_ptr<RankedEvaluator> evaluator;

  TrueSkillTeam(const std::shared_ptr<RankedTeam>& cTeam, const std::shared_ptr<RankedEvaluator>& cEvaluator);
};

#endif /* TRUESKILL_H */