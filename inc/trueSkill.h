#ifndef TRUESKILL_H
#define TRUESKILL_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>
#include <boost/array.hpp>
#include <ostream>

class league;
class ranked_team;
class ranked;
class trueSkill;
class trueSkillTeam;
struct heatResult;
struct gameResult;


class trueSkillSettings
{
public:
  /* mean of prior skill belief */
  fpType initialMean;

  /* standard deviation of prior skill belief */
  fpType initialStdDev;

  /* standard deviation of performance; the difference in points necessary to assert
  a team will have an 80% : 20% ratio of winning : losing */
  fpType performanceStdDev;

  /* probability of a draw given two equal teams */
  fpType drawProbability;

  /* speed of rank propagation */
  fpType dynamicsFactor;

private:
  /* equivalent to performanceStdDev squared */
  fpType betaSquared;

  fpType drawMargin;

  fpType tauSquared;
public:

  static const trueSkillSettings defaultSettings;
  static const fpType defaultMean;
  static const fpType defaultStdDev;
  static const fpType defaultPerfDev;
  static const fpType defaultDrawProb;
  static const fpType defaultDynamicsFactor;

  trueSkillSettings(const trueSkillSettings& other);
  trueSkillSettings(
    fpType _iMean = defaultMean, 
    fpType _iStdDev = defaultStdDev, 
    fpType _dPerf = defaultPerfDev, 
    fpType _pDraw = defaultDrawProb, 
    fpType _dFactor = defaultDynamicsFactor);

  friend class trueSkill;
};

class trueSkill
{
private:
  /* measure of the mean of a given object's skill */
  fpType mean;

  /* measure of the variance of a given object's skill */
  fpType stdDev;


  static void teamRank_finalize(trueSkill& cResult, fpType currentProportion, fpType totalProportion, const trueSkillSettings& settings);

public:
  trueSkill(const trueSkill& other)
    : mean(other.mean),
    stdDev(other.stdDev)
  {
  }

  trueSkill(const trueSkillSettings& settings = trueSkillSettings::defaultSettings)
    : mean(settings.initialMean),
    stdDev(settings.initialStdDev)
  {
  };

  trueSkill(fpType _mean, fpType _stdDev)
    : mean(_mean),
    stdDev(_stdDev)
  {
  };

  trueSkill(size_t numTeammates, const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  void output(std::ostream& oFile, bool printHeader = true) const;

  bool input(const std::vector<std::string>& lines, size_t& firstLine);

  fpType getMean() const
  {
    return mean;
  };

  fpType getStdDev() const
  {
    return stdDev;
  };

  void setMean(fpType _mean)
  {
    mean = _mean;
  };

  void setStdDev(fpType _stdDev)
  {
    stdDev = _stdDev;
  };

  fpType getRank() const
  {
    return mean - stdDev * 3.0;
  };

  bool operator<(const trueSkill& other) const
  {
    return getRank() < other.getRank();
  };

  bool operator>(const trueSkill& other) const
  {
    return getRank() > other.getRank();
  };

  static fpType calculateDrawMargin(fpType drawProbability, fpType beta);

  /* determine if performance is greater than draw margin */
  static fpType VExceedsMargin(fpType dPerformance, fpType drawMargin);
  static fpType WExceedsMargin(fpType dPerformance, fpType drawMargin);

  /* determine if performance is within margin */
  static fpType VWithinMargin(fpType dPerformance, fpType drawMargin);
  static fpType WWithinMargin(fpType dPerformance, fpType drawMargin);

  static size_t update(
    trueSkillTeam& team_A,
    trueSkillTeam& team_B,
    const gameResult& gResult,
    const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  static void update_ranked(
    trueSkill& cSkill, 
    fpType v, 
    fpType w, 
    fpType c, 
    fpType cSquared, 
    fpType rankMultiplier, 
    fpType partialPlay, 
    const trueSkillSettings& s = trueSkillSettings::defaultSettings);

  /* determine the quality of a match between two teams */
  static fpType matchQuality(
    const trueSkillTeam& team_A, 
    const trueSkillTeam& team_B,
    const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  /* estimate the rank of a team given its component teams */
  template<class constRankedIterator_t, class constDoubleIterator_t>
  static trueSkill teamRank(
    constRankedIterator_t iComponent, 
    constRankedIterator_t componentEnd, 
    constDoubleIterator_t iProportion, 
    fpType totalProportion = 1.0, 
    const trueSkillSettings& tSettings = trueSkillSettings::defaultSettings)
  {
    fpType currentProportion = 0;
    trueSkill cResult(0, 0);

    for ( ;iComponent != componentEnd; ++iComponent, ++iProportion)
    {
      const trueSkill& cSkill = (*iComponent)->getSkill();

      cResult.mean += cSkill.mean * (*iProportion / totalProportion);
      cResult.stdDev += (cSkill.stdDev*cSkill.stdDev) * (*iProportion / totalProportion);

      currentProportion += *iProportion;
    }

    teamRank_finalize(cResult, currentProportion, totalProportion, tSettings);
    return cResult;
  }; // endOf teamRank

  /* adds a margin to the std dev due to a team's being mutated */
  void feather(const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  friend class trueSkillSettings;
  friend class pkIO;
};

class trueSkillTeam
{
public:
  ranked_team* baseTeam;
  ranked* baseEvaluator;
  trueSkill aggregateSkill;
  std::vector<ranked_team*> subTeams;
  std::vector< boost::array<size_t, 6> > correspondencies;

  trueSkillTeam();
  trueSkillTeam(ranked_team& cTeam, ranked& cEvaluator, const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  void push_back(ranked_team& teammate, boost::array<size_t, 6>& _correspondencies)
  {
    subTeams.push_back(&teammate);
    correspondencies.push_back(_correspondencies);
  };

  void finalize(const trueSkillSettings& settings = trueSkillSettings::defaultSettings);
};

#endif /* TRUESKILL_H */