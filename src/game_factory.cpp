#include "../inc/game_factory.h"

double GameFactory::calculateDrawMargin(double drawProbability, double beta)
{
  static boost::math::normal_distribution<double> cNormal;

  double margin = boost::math::cdf( boost::math::complement(cNormal, 0.5 * (drawProbability) + 1) )
    * sqrt((double)2.0)
    * beta;

  return margin;
};





double GameFactory::VExceedsMargin(double dPerformance, double drawMargin)
{
  static boost::math::normal_distribution<double> cNormal;

  double denominator = boost::math::cdf(cNormal, dPerformance - drawMargin);

  if (denominator < 2.222758749e-162)
  {
    return -dPerformance + drawMargin;
  }

  return boost::math::pdf(cNormal, (dPerformance - drawMargin)) / denominator;

}





double GameFactory::WExceedsMargin(double dPerformance, double drawMargin)
{
  static boost::math::normal_distribution<double> cNormal;

  double denominator = boost::math::cdf(cNormal, dPerformance - drawMargin);

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

  double vWin = VExceedsMargin(dPerformance, drawMargin);
  vWin = vWin * (vWin + dPerformance - drawMargin);
  return vWin;
}





double GameFactory::VWithinMargin(double dPerformance, double drawMargin)
{
  static boost::math::normal_distribution<double> cNormal;

  double dPerformanceAbs = fastabs(dPerformance);

  double denominator =
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

  double numerator =
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





double GameFactory::WWithinMargin(double dPerformance, double drawMargin)
{
  static boost::math::normal_distribution<double> cNormal;

  double dPerformanceAbs = fastabs(dPerformance);
  double drawMinusPerformanceAbs = drawMargin - dPerformanceAbs;
  double negDrawMinusPerformanceAbs = -drawMargin - dPerformanceAbs;

  double denominator =
    boost::math::cdf(cNormal, drawMinusPerformanceAbs)
    -
    boost::math::cdf(cNormal, negDrawMinusPerformanceAbs);

  if (denominator < 2.222758749e-162)
  {
    return 1.0;
  }

  double vt = VWithinMargin(dPerformanceAbs, drawMargin);

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





double GameFactory::matchQuality(
  const Battlegroup& team_A,
  const Battlegroup& team_B)
{
  // total number of teammates on the team
  size_t totalTeammates = team_A.team()->nv().getNumTeammates() * 2 + team_B.team()->nv().getNumTeammates() * 2;
  const TrueSkill& skillA = team_A.skill();
  const TrueSkill& skillB = team_B.skill();

  double skillAStdDevSquared = pow(skillA.stdDev , 2);
  double skillBStdDevSquared = pow(skillB.stdDev , 2);

  double denominator = ((totalTeammates * betaSquared_) + skillAStdDevSquared + skillBStdDevSquared);

  double sqrtPart =
    sqrt
    (
    (totalTeammates * betaSquared_)
    /
    denominator
    );

  double expPart =
    exp
    (
    (-1 * pow(skillA.mean - skillB.mean, 2))
    /
    ( 2 * denominator)
    );

  double result = expPart * sqrtPart;
  assert(!boost::math::isnan(result));
  return result;
}; //endOf matchQuality





size_t GameFactory::update(
  TrueSkillTeam& team_A,
  TrueSkillTeam& team_B,
  const HeatResult& hResult)
{
  int outcome = gResult.endStatus;
  std::array< TrueSkillTeam*, 2 > teams =
    {{ &team_A, &team_B }};
  std::array<TrueSkill*, 2> ratings =
    {{ &team_A.aggregateSkill , &team_B.aggregateSkill }};
  std::array<double, 2> rankMultiplier;

  double c, cSquared, v, w, winningMean, losingMean, meanDelta;

  {
    size_t totalPlayers = team_A.baseTeam->team.getNumTeammates() * 2 + team_B.baseTeam->team.getNumTeammates() * 2;
    double ratingsSquared =
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
    double totalContribution = 0;
    size_t tNumTeammates = tTeam.baseTeam->team.getNumTeammates();
    for (size_t iTeammate = 0; iTeammate != tNumTeammates; ++iTeammate)
    {
      //double cContribution = gResult.aggregateContribution[iTeam][iTeammate];
      totalContribution += gResult.aggregateContribution[iTeam][iTeammate];
    }

    // update component teams:
    /*size_t iComponent = 0;
    BOOST_FOREACH(ranked_team* componentTeam, tTeam.subTeams)
    {
      const std::array<size_t, 6>& correspondence = tTeam.correspondencies[iComponent];
      size_t oNumTeammates = componentTeam->team.getNumTeammates();
      // find contribution of this team:
      double contribution = 0;

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

void GameFactory::update_ranked(
  TrueSkill& cSkill,
  double v,
  double w,
  double c,
  double cSquared,
  double rankMultiplier,
  double partialPlay,
  const trueSkillSettings& s)
{
  double oMeanMultiplier = partialPlay * ((pow(cSkill.getStdDev(), 2) + s.tauSquared) / c);

  double oStdDevMultiplier = partialPlay * ((pow(cSkill.getStdDev(), 2) + s.tauSquared) / cSquared);

  double oTeamMeanDelta = (rankMultiplier * oMeanMultiplier * v);

  double oNewMean = cSkill.getMean() + oTeamMeanDelta;
  double oNewStdDev =
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