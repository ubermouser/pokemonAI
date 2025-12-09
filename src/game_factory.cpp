#include "pokemonai/game_factory.h"

#include <algorithm>
#include <boost/math/distributions/normal.hpp>
#include <numeric>
#include <unordered_set>

GameFactory::GameFactory(const Config& cfg)
  : cfg_(cfg),
    betaSquared_(pow(TrueSkill::performanceStdDev(), 2)),
    drawMargin_(calculateDrawMargin(cfg_.drawProbability, TrueSkill::performanceStdDev())),
    tauSquared_(pow(cfg.dynamicsFactor, 2)) {

}


double GameFactory::calculateDrawMargin(double drawProbability, double beta) {
  static boost::math::normal_distribution<double> cNormal;

  double margin = boost::math::cdf( boost::math::complement(cNormal, 0.5 * (drawProbability) + 1) )
    * sqrt((double)2.0)
    * beta;

  return margin;
};


double GameFactory::VExceedsMargin(double dPerformance, double drawMargin) {
  static boost::math::normal_distribution<double> cNormal;

  double denominator = boost::math::cdf(cNormal, dPerformance - drawMargin);

  if (denominator < 2.222758749e-162) {
    return -dPerformance + drawMargin;
  }

  return boost::math::pdf(cNormal, (dPerformance - drawMargin)) / denominator;

}


double GameFactory::WExceedsMargin(double dPerformance, double drawMargin) {
  static boost::math::normal_distribution<double> cNormal;

  double denominator = boost::math::cdf(cNormal, dPerformance - drawMargin);

  if (denominator < 2.222758749e-162) {
    if (dPerformance < 0.0) {
      return 1.0;
    } else {
      return 0.0;
    }
  }

  double vWin = VExceedsMargin(dPerformance, drawMargin);
  vWin = vWin * (vWin + dPerformance - drawMargin);
  return vWin;
}


double GameFactory::VWithinMargin(double dPerformance, double drawMargin) {
  static boost::math::normal_distribution<double> cNormal;

  double dPerformanceAbs = fastabs(dPerformance);

  double denominator =
    boost::math::cdf(cNormal, drawMargin - dPerformanceAbs)
    -
    boost::math::cdf(cNormal, -drawMargin - dPerformanceAbs);

  if (denominator < 2.222758749e-162) {
    if (dPerformance < 0.0) {
      return -dPerformance - drawMargin;
    } else {
      return -dPerformance + drawMargin;
    }
  }

  double numerator =
    boost::math::pdf(cNormal, -drawMargin - dPerformanceAbs)
    -
    boost::math::pdf(cNormal, drawMargin - dPerformanceAbs);

  if (dPerformance < 0.0) {
    return -1 * numerator / denominator;
  } else {
    return numerator / denominator;
  }
}


double GameFactory::WWithinMargin(double dPerformance, double drawMargin) {
  static boost::math::normal_distribution<double> cNormal;

  double dPerformanceAbs = fastabs(dPerformance);
  double drawMinusPerformanceAbs = drawMargin - dPerformanceAbs;
  double negDrawMinusPerformanceAbs = -drawMargin - dPerformanceAbs;

  double denominator =
    boost::math::cdf(cNormal, drawMinusPerformanceAbs)
    -
    boost::math::cdf(cNormal, negDrawMinusPerformanceAbs);

  if (denominator < 2.222758749e-162) {
    return 1.0;
  }

  double vt = VWithinMargin(dPerformanceAbs, drawMargin);

  return
    vt * vt
    +
    (
      (drawMinusPerformanceAbs)    * boost::math::pdf(cNormal, drawMinusPerformanceAbs)
      -
      (negDrawMinusPerformanceAbs) * boost::math::pdf(cNormal, negDrawMinusPerformanceAbs)
    )
    /
    denominator;
}


double GameFactory::matchQuality(
  const Battlegroup& team_A,
  const Battlegroup& team_B) const {
  const TrueSkill& skillA = team_A.skill();
  const TrueSkill& skillB = team_B.skill();

  double skillAStdDevSquared = skillA.variance;
  double skillBStdDevSquared = skillB.variance;

  double denominator =
      (betaSquared_ + skillAStdDevSquared + skillBStdDevSquared);

  double sqrtPart = sqrt(betaSquared_ / denominator);

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


std::unordered_set<uint64_t> findCommonContributions(
    const std::vector<GroupContribution>& team_a,
    const std::vector<GroupContribution>& team_b) {
  auto createUnorderedSet = [](const auto& vec) {
    std::unordered_set<uint64_t> result;
    for (const GroupContribution& cnt: vec) { result.insert(cnt.identity); }
    return result;
  };
  auto set_a = createUnorderedSet(team_a);
  auto set_b = createUnorderedSet(team_b);

  // find common elements:
  std::unordered_set<uint64_t> result;
  for (uint64_t id: set_a) {
    if (set_b.count(id) > 0) { result.insert(id); }
  }

  return result;
}


size_t GameFactory::update(
  Battlegroup& team_A,
  Battlegroup& team_B,
  const HeatResult& hResult) const {
  int outcome = hResult.endStatus;
  std::array<Battlegroup*, 2 > teams{&team_A, &team_B};
  std::array<TrueSkill*, 2> ratings{ &team_A.record().skill , &team_B.record().skill };
  std::array<std::vector<GroupContribution>, 2> contributions{team_A.contributions(), team_B.contributions()};
  std::array<double, 2> rankMultiplier;
  std::unordered_set<uint64_t> commonContributions =
      findCommonContributions(contributions[0], contributions[1]);

  double c, cSquared, v, w, winningMean, losingMean, meanDelta;

  {
    double ratingsSquared = ratings[TEAM_A]->variance + ratings[TEAM_B]->variance;

    cSquared = ratingsSquared + betaSquared_;

    c = sqrt(cSquared);
  }

  // determine multipliers:
  rankMultiplier[TEAM_A] = (outcome==MATCH_TEAM_B_WINS)?-1:1;
  rankMultiplier[TEAM_B] = (outcome==MATCH_TEAM_A_WINS)?-1:1;

  // determine dMean:
  switch(outcome) {
  default:
  case MATCH_TEAM_A_WINS:
  case MATCH_DRAW:
  case MATCH_TIE:
    winningMean = ratings[TEAM_A]->mean;
    losingMean = ratings[TEAM_B]->mean;
    break;
  case MATCH_TEAM_B_WINS:
    winningMean = ratings[TEAM_B]->mean;
    losingMean = ratings[TEAM_A]->mean;
    break;
  }
  meanDelta = winningMean - losingMean;

  // determine probability of outcome:
  switch(outcome) {
  default:
  case MATCH_TEAM_A_WINS: // win or lose:
  case MATCH_TEAM_B_WINS:
    v = VExceedsMargin(meanDelta / c, drawMargin_ / c);
    w = WExceedsMargin(meanDelta / c, drawMargin_ / c);
    break;
  case MATCH_DRAW: // draw or tie case:
  case MATCH_TIE:
    v = VWithinMargin(meanDelta / c, drawMargin_ / c);
    w = WWithinMargin(meanDelta / c, drawMargin_ / c);
    break;
  }

  size_t numUpdates = 0;
  //bool evaluatorsSame = *(teams[0]->baseEvaluator) == *(teams[1]->baseEvaluator);
  for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
    Battlegroup& tTeam = *teams[iTeam];
    std::vector<GroupContribution>& contribs = contributions[iTeam];

    for (auto& cnt: contribs) {
      // don't update a contribution that is both a winner and a loser at the same time:
      if (commonContributions.count(cnt.identity) > 0) { continue; }
      // update contribution:
      update_ranked(cnt.skill, v, w, c, cSquared, rankMultiplier[iTeam], cnt.contribution);
      numUpdates++;
    }
    // recompute battlegroup skill:
    tTeam.computeSkill();
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
    double partialPlay) const {
  double oMeanMultiplier = partialPlay * ((cSkill.variance + tauSquared_) / c);

  double oStdDevMultiplier = partialPlay * ((cSkill.variance + tauSquared_) / cSquared);

  double oTeamMeanDelta = (rankMultiplier * oMeanMultiplier * v);

  double oNewMean = cSkill.mean + oTeamMeanDelta;
  double oNewVariance =
        (cSkill.variance + tauSquared_)
        *
        (1 - w * oStdDevMultiplier);

  assert(!boost::math::isnan(oNewMean) || !boost::math::isnan(oNewVariance));

  cSkill.mean = oNewMean;
  cSkill.variance = oNewVariance;
}; // endOf update_ranked
