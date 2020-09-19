#include "../inc/true_skill.h"

#include <vector>
#include <array>
#include <boost/math/distributions/normal.hpp>
#include <boost/foreach.hpp>
#include <ostream>
#include <iomanip>
#include <math.h>

#include "../inc/ranked.h"
#include "../inc/ranked_team.h"
#include "../inc/game.h"
#include "../inc/fp_compare.h"
#include "../inc/init_toolbox.h"

const trueSkillSettings trueSkillSettings::defaultSettings = trueSkillSettings();

const fpType trueSkillSettings::defaultMean = 25.0;
const fpType trueSkillSettings::defaultStdDev = defaultMean / 3.0;
const fpType trueSkillSettings::defaultPerfDev = defaultMean / 6.0;
const fpType trueSkillSettings::defaultDrawProb = 0.10;
const fpType trueSkillSettings::defaultDynamicsFactor = defaultMean / 300.0;

trueSkillSettings::trueSkillSettings(fpType _iMean, fpType _iStdDev, fpType _dPerf, fpType _pDraw, fpType _dFactor)
  : initialMean(_iMean),
  initialStdDev(_iStdDev),
  performanceStdDev(_dPerf),
  drawProbability(_pDraw),
  dynamicsFactor(_dFactor),
  betaSquared(pow(performanceStdDev, 2)),
  drawMargin(TrueSkill::calculateDrawMargin(drawProbability, performanceStdDev)),
  tauSquared(pow(dynamicsFactor, 2))
{
};

trueSkillSettings::trueSkillSettings(const trueSkillSettings& other)
  : initialMean(other.initialMean),
  initialStdDev(other.initialStdDev),
  performanceStdDev(other.performanceStdDev),
  drawProbability(other.drawProbability),
  dynamicsFactor(other.dynamicsFactor),
  betaSquared(pow(performanceStdDev, 2)),
  drawMargin(TrueSkill::calculateDrawMargin(drawProbability, performanceStdDev)),
  tauSquared(pow(dynamicsFactor, 2))
{
};





TrueSkillTeam::TrueSkillTeam()
  : baseTeam(NULL),
  baseEvaluator(NULL),
  aggregateSkill(),
  subTeams(),
  correspondencies()
{
};

TrueSkillTeam::TrueSkillTeam(RankedTeam& cTeam, Ranked& cEvaluator, const trueSkillSettings& settings)
  : baseTeam(&cTeam),
  baseEvaluator(&cEvaluator),
  aggregateSkill(settings),
  subTeams(),
  correspondencies()
{
  finalize(settings);
};

void TrueSkillTeam::finalize(const trueSkillSettings& settings)
{
  static const std::array<fpType, 2> proportion = {{ 1.0, 1.0 }};

  const std::array<const Ranked*, 2> components = {{ baseEvaluator, baseTeam }};
  // scale evaluation function's trueskill by the team's size: (evaluation function is always half the total trueskill)
  //proportion[0] *= baseTeam->team.getNumTeammates();

  aggregateSkill = TrueSkill::teamRank(components.begin(), components.end(), proportion.begin(), 1.0, settings);
};





TrueSkill::TrueSkill(size_t numTeammates, const trueSkillSettings& settings)
  : mean(settings.initialMean * numTeammates),
  stdDev(sqrt(pow(settings.initialStdDev, 2) * numTeammates))
{
}




static const std::string header = "PKATS0";

void TrueSkill::output(std::ostream& oFile, bool printHeader) const
{
  // header:
  if (printHeader)
  {
    oFile << header << "\t";
  };

  oFile 
    << std::setprecision(20) << getMean() 
    << "\t" << std::setprecision(20) << getStdDev() 
    << "\n";
};

bool TrueSkill::input(const std::vector<std::string>& lines, size_t& iLine)
{
  // are the enough lines in the input stream:
  if ((lines.size() - iLine) < 1U)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": unexpected end of input stream at line " << iLine << "!\n";
    return false; 
  }

  // compare trueskill header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": trueskill stream has header of type \"" << lines.at(0).substr(0, header.size()) << 
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // input trueskill:
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)3, (size_t)3)) { return false; }

    if (!INI::setArgAndPrintError("trueskill mean", tokens.at(1), mean, iLine, 1)) { return false; }
    if (!INI::setArgAndPrintError("trueskill stdDev", tokens.at(2), stdDev, iLine, 2)) { return false; }
  }
  iLine++;
  return true;
};





fpType TrueSkill::calculateDrawMargin(fpType drawProbability, fpType beta)
{
  static boost::math::normal_distribution<fpType> cNormal;

  fpType margin = boost::math::cdf( boost::math::complement(cNormal, 0.5 * (drawProbability) + 1) )
    * sqrt((fpType)2.0)
    * beta;

  return margin;
};





fpType TrueSkill::VExceedsMargin(fpType dPerformance, fpType drawMargin)
{
  static boost::math::normal_distribution<fpType> cNormal;

  fpType denominator = boost::math::cdf(cNormal, dPerformance - drawMargin);

  if (denominator < 2.222758749e-162)
  {
    return -dPerformance + drawMargin;
  }

  return boost::math::pdf(cNormal, (dPerformance - drawMargin)) / denominator;

}





fpType TrueSkill::WExceedsMargin(fpType dPerformance, fpType drawMargin)
{
  static boost::math::normal_distribution<fpType> cNormal;

  fpType denominator = boost::math::cdf(cNormal, dPerformance - drawMargin);

  if (denominator < 2.222758749e-162)
  {
    if (dPerformance < 0.0)
    {
      return 1.0;
    }
    else
    {
      return 0.0;
    }
  }

  fpType vWin = VExceedsMargin(dPerformance, drawMargin);
  vWin = vWin * (vWin + dPerformance - drawMargin);
  return vWin;
}





fpType TrueSkill::VWithinMargin(fpType dPerformance, fpType drawMargin)
{
  static boost::math::normal_distribution<fpType> cNormal;

  fpType dPerformanceAbs = fastabs(dPerformance);

  fpType denominator = 
    boost::math::cdf(cNormal, drawMargin - dPerformanceAbs)
    -
    boost::math::cdf(cNormal, -drawMargin - dPerformanceAbs);

  if (denominator < 2.222758749e-162)
  {
    if (dPerformance < 0.0)
    {
      return -dPerformance - drawMargin;
    }
    else
    {
      return -dPerformance + drawMargin;
    }
  }

  fpType numerator = 
    boost::math::pdf(cNormal, -drawMargin - dPerformanceAbs)
    - 
    boost::math::pdf(cNormal, drawMargin - dPerformanceAbs);

  if (dPerformance < 0.0)
  {
    return -1 * numerator / denominator;
  }
  else
  {
    return numerator / denominator;
  }
}





fpType TrueSkill::WWithinMargin(fpType dPerformance, fpType drawMargin)
{
  static boost::math::normal_distribution<fpType> cNormal;

  fpType dPerformanceAbs = fastabs(dPerformance);
  fpType drawMinusPerformanceAbs = drawMargin - dPerformanceAbs;
  fpType negDrawMinusPerformanceAbs = -drawMargin - dPerformanceAbs;

  fpType denominator = 
    boost::math::cdf(cNormal, drawMinusPerformanceAbs) 
    -
    boost::math::cdf(cNormal, negDrawMinusPerformanceAbs);

  if (denominator < 2.222758749e-162)
  {
    return 1.0;
  }

  fpType vt = VWithinMargin(dPerformanceAbs, drawMargin);

  return 
    vt * vt
    +
    (
      (drawMinusPerformanceAbs)
      *
      boost::math::pdf(cNormal, drawMinusPerformanceAbs)
      -
      (negDrawMinusPerformanceAbs)
      *
      boost::math::pdf(cNormal, negDrawMinusPerformanceAbs)
    )
    /
    denominator;
}





fpType TrueSkill::matchQuality(
  const TrueSkillTeam& team_A, 
  const TrueSkillTeam& team_B,
  const trueSkillSettings& settings)
{
  // total number of teammates on the team
  size_t totalTeammates = team_A.baseTeam->team.getNumTeammates() * 2 + team_B.baseTeam->team.getNumTeammates() * 2;
  const TrueSkill& skillA = team_A.aggregateSkill;
  const TrueSkill& skillB = team_B.aggregateSkill;

  fpType skillAStdDevSquared = pow(skillA.getStdDev() , 2);
  fpType skillBStdDevSquared = pow(skillB.getStdDev() , 2);

  fpType denominator = ((totalTeammates * settings.betaSquared) + skillAStdDevSquared + skillBStdDevSquared);

  fpType sqrtPart =
    sqrt
    (
    (totalTeammates * settings.betaSquared)
    /
    denominator
    );

  fpType expPart =
    exp
    (
    (-1 * pow(skillA.getMean() - skillB.getMean(), 2))
    /
    ( 2 * denominator)
    );

  fpType result = expPart * sqrtPart;
  assert(!boost::math::isnan(result));
  return result;
}; //endOf matchQuality





size_t TrueSkill::update(
  TrueSkillTeam& team_A,
  TrueSkillTeam& team_B,
  const GameResult& gResult,
  const trueSkillSettings& s)
{
  int outcome = gResult.endStatus;
  std::array< TrueSkillTeam*, 2 > teams = 
    {{ &team_A, &team_B }};
  std::array<TrueSkill*, 2> ratings = 
    {{ &team_A.aggregateSkill , &team_B.aggregateSkill }};
  std::array<fpType, 2> rankMultiplier;

  fpType c, cSquared, v, w, winningMean, losingMean, meanDelta;
  
  {
    size_t totalPlayers = team_A.baseTeam->team.getNumTeammates() * 2 + team_B.baseTeam->team.getNumTeammates() * 2;
    fpType ratingsSquared = 
      pow(ratings[TEAM_A]->getStdDev(), 2)
      +
      pow(ratings[TEAM_B]->getStdDev(), 2);

    cSquared = ratingsSquared + totalPlayers * s.betaSquared;

    c = sqrt(cSquared);
  }

  // determine multipliers:
  rankMultiplier[TEAM_A] = (outcome==MATCH_TEAM_B_WINS)?-1:1;
  rankMultiplier[TEAM_B] = (outcome==MATCH_TEAM_A_WINS)?-1:1;

  // determine dMean:
  switch(outcome)
  {
  default:
  case MATCH_TEAM_A_WINS:
  case MATCH_DRAW:
  case MATCH_TIE:
    winningMean = ratings[TEAM_A]->getMean();
    losingMean = ratings[TEAM_B]->getMean();
    break;
  case MATCH_TEAM_B_WINS:
    winningMean = ratings[TEAM_B]->getMean();
    losingMean = ratings[TEAM_A]->getMean();
    break;
  }
  meanDelta = winningMean - losingMean;

  // determine probability of outcome:
  switch(outcome)
  {
  default:
  case MATCH_TEAM_A_WINS: // win or lose:
  case MATCH_TEAM_B_WINS:
    v = VExceedsMargin(meanDelta / c, s.drawMargin / c);
    w = WExceedsMargin(meanDelta / c, s.drawMargin / c);
    break;
  case MATCH_DRAW: // draw or tie case:
  case MATCH_TIE:
    v = VWithinMargin(meanDelta / c, s.drawMargin / c);
    w = WWithinMargin(meanDelta / c, s.drawMargin / c);
    break;
  }

  size_t numUpdates = 0;
  bool evaluatorsSame = *(teams[0]->baseEvaluator) == *(teams[1]->baseEvaluator);
  for (size_t iTeam = 0; iTeam != 2; ++iTeam)
  {
    TrueSkillTeam& tTeam = *teams[iTeam];
    
    // update team:
    update_ranked(tTeam.baseTeam->getSkill(), v, w, c, cSquared, rankMultiplier[iTeam], evaluatorsSame?1.0:0.5, s);
    numUpdates++;

    // if evaluators are not the same, update evaluators:
    if (!evaluatorsSame)
    {
      update_ranked(tTeam.baseEvaluator->getSkill(), v, w, c, cSquared, rankMultiplier[iTeam], 0.5, s);
      numUpdates++;
    }

    // find total contribution:
    fpType totalContribution = 0;
    size_t tNumTeammates = tTeam.baseTeam->team.getNumTeammates();
    for (size_t iTeammate = 0; iTeammate != tNumTeammates; ++iTeammate)
    {
      //fpType cContribution = gResult.aggregateContribution[iTeam][iTeammate];
      totalContribution += gResult.aggregateContribution[iTeam][iTeammate];
    }

    // update component teams:
    /*size_t iComponent = 0;
    BOOST_FOREACH(ranked_team* componentTeam, tTeam.subTeams)
    {
      const std::array<size_t, 6>& correspondence = tTeam.correspondencies[iComponent];
      size_t oNumTeammates = componentTeam->team.getNumTeammates();
      // find contribution of this team:
      fpType contribution = 0;

      for (size_t iOTeammate = 0; iOTeammate < oNumTeammates; ++iOTeammate)
      {
        // no match; teammate not in subteam
        assert(correspondence[iOTeammate] != SIZE_MAX);

        // add ranking to this subteam's total:
        contribution += gResult.aggregateContribution[iTeam][correspondence[iOTeammate]];
      }
      // now scaled from 0..1 - 0 is the minimum possible contribution, 1 is the maxmimum (and default) contribution
      contribution = scale( contribution, totalContribution, 0.0);
      // prevent negative contributions -- even if a teammate contributes in the opposite manner as the rest of his team,
      // that teammates update is in the same direction as the team's update (however minor)
      // this ordering guarantees nan will not be propagated, and that 1.0 is the default
      contribution = std::max(0.05, std::min(1.0, contribution));

      // perform update:
      update_ranked(componentTeam->getSkill(), v, w, c, cSquared, rankMultiplier[iTeam], contribution * evaluatorsSame?1.0:0.5 , s);

      numUpdates++;
      iComponent++;
    }*/
  } // end of for each team

  return numUpdates;
} // endOf update

void TrueSkill::update_ranked(
  TrueSkill& cSkill, 
  fpType v, 
  fpType w, 
  fpType c, 
  fpType cSquared, 
  fpType rankMultiplier, 
  fpType partialPlay, 
  const trueSkillSettings& s)
{
  fpType oMeanMultiplier = partialPlay * ((pow(cSkill.getStdDev(), 2) + s.tauSquared) / c);

  fpType oStdDevMultiplier = partialPlay * ((pow(cSkill.getStdDev(), 2) + s.tauSquared) / cSquared);

  fpType oTeamMeanDelta = (rankMultiplier * oMeanMultiplier * v);

  fpType oNewMean = cSkill.getMean() + oTeamMeanDelta;
  fpType oNewStdDev = 
    sqrt(
        (
          pow(cSkill.getStdDev(), 2) 
          + 
          s.tauSquared
        )
        *
        (1 - w*oStdDevMultiplier)
      );
          

  assert(!boost::math::isnan(oNewMean) || !boost::math::isnan(oNewStdDev));

  cSkill.setMean(oNewMean);
  cSkill.setStdDev(oNewStdDev);
}; // endOf update_ranked





void TrueSkill::teamRank_finalize(TrueSkill& cResult, fpType currentProportion, fpType totalProportion, const trueSkillSettings& settings)
{
  if (mostlyLT(currentProportion, totalProportion))
  {
    fpType leftover = totalProportion - currentProportion;

    cResult.mean += settings.initialMean * leftover;
    cResult.stdDev += pow(settings.initialStdDev, 2) * leftover;
  }

  cResult.stdDev = sqrt(cResult.stdDev);
}; // endOf teamRank_finalize





void TrueSkill::feather(const trueSkillSettings& settings)
{
  stdDev = std::max(sqrt(pow(stdDev, 2) * 2), std::min(stdDev, settings.initialStdDev));
};
