#ifndef TRUESKILL_H
#define TRUESKILL_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>
#include <array>
#include <ostream>
#include <boost/property_tree/ptree.hpp>


struct TrueSkill {
  /* measure of the mean of a given object's skill */
  double mean = initialMean();

  /* measure of the variance of a given object's skill */
  double stdDev = initialStdDev();

  TrueSkill(double mean_ = initialMean(), double stdDev_ = initialStdDev()) : mean(mean_), stdDev(stdDev_) { }

  /* mean of prior skill belief */
  static constexpr double initialMean() { return 25.0; }

  /* standard deviation of prior skill belief */
  static constexpr double initialStdDev() { return initialMean() / 3.0; }

  /* standard deviation of performance; the difference in points necessary to assert
  a team will have an 80% : 20% ratio of winning : losing */
  static constexpr double performanceStdDev() { return initialMean() / 6.0; }

  double rank() const { return mean - (stdDev * 3.0); }
  
  bool operator <(const TrueSkill& other) const { return rank() < other.rank(); };
  bool operator >(const TrueSkill& other) const { return rank() > other.rank(); };

  std::ostream& print(std::ostream& os) const;

  boost::property_tree::ptree output() const;
  void input(const boost::property_tree::ptree& ptree);
};

#endif /* TRUESKILL_H */