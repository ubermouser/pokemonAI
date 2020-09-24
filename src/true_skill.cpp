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





TrueSkill::TrueSkill(size_t numTeammates)
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
