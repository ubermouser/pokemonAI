#ifndef TRUESKILL_H
#define TRUESKILL_H

#include "pkai.h"

#include <iosfwd>
#include <boost/property_tree/ptree_fwd.hpp>
#include <vector>


struct GroupContribution;


struct TrueSkill {
  /* measure of the mean of a given object's skill */
  double mean;

  /* measure of the variance of a given object's skill */
  double variance;

  explicit TrueSkill(double mean_ = initialMean(), double variance_ = initialVariance()) : mean(mean_), variance(variance_) { }

  /* mean of prior skill belief */
  static constexpr double initialMean() { return 25.0; }

  /* standard deviation of prior skill belief */
  static constexpr double initialVariance() { return (initialMean() * initialMean()) / 9.0; }

  /* standard deviation of performance; the difference in points necessary to assert
  a team will have an 80% : 20% ratio of winning : losing */
  static constexpr double performanceStdDev() { return initialMean() / 6.0; }

  /* A trueskill containing zero mean and stddev. Useful for accumulation */
  static TrueSkill zero() { return TrueSkill{0.0, 0.0}; }

  /* Combines  set of trueskills by their contribution amount */
  static TrueSkill combine(const std::vector<GroupContribution>& contributions);

  double rank() const { return mean - (stdDev() * 3.0); }
  double stdDev() const;
  double precision() const { return 1. / variance; }
  double precisionAdjustedMean() const { return mean / variance; }
  
  bool operator <(const TrueSkill& other) const { return rank() < other.rank(); };
  bool operator >(const TrueSkill& other) const { return rank() > other.rank(); };

  // multiply by scalar (contribution):
  friend TrueSkill operator *(TrueSkill lhs, double rhs) { lhs *=  rhs; return lhs; }
  TrueSkill& operator *=(double rhs) {
    mean *= rhs;
    variance *= rhs;
    return *this;
  }

  // accumulate trueskills in a team::
  friend TrueSkill operator +(TrueSkill lhs, const TrueSkill& rhs) { lhs += rhs; return lhs; }
  TrueSkill& operator +=(const TrueSkill& rhs) {
    mean += rhs.mean;
    variance += rhs.variance;
    return *this;
  }

  std::ostream& print(std::ostream& os) const;

  boost::property_tree::ptree output() const;
  void input(const boost::property_tree::ptree& ptree);
};


struct GroupContribution {
  TrueSkill& skill;
  double contribution;
};


std::ostream& operator <<(std::ostream& os, const TrueSkill& tR);

#endif /* TRUESKILL_H */