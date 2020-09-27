#include "../inc/true_skill.h"

#include <cmath>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <complex>
#include <ostream>


namespace pt = boost::property_tree;


double TrueSkill::stdDev() const {
  return std::sqrt(variance);
}


TrueSkill TrueSkill::combine(const std::vector<GroupContribution>& contributions) {
  TrueSkill result = TrueSkill::zero();
  for (auto& cnt: contributions) {
    result += cnt.skill * cnt.contribution;
  }

  return result;
}


pt::ptree TrueSkill::output() const {
  pt::ptree result;
  result.put("mean", mean);
  result.put("variance", variance);

  return result;
};


void TrueSkill::input(const pt::ptree& tree) {
  mean = tree.get<double>("mean");
  variance = tree.get<double>("variance");
};


std::ostream& TrueSkill::print(std::ostream& os) const {
  os << boost::format("m=%6.2f s=%5.2f") % mean % stdDev();
  return os;
}


std::ostream& operator <<(std::ostream& os, const TrueSkill& tR) {
  tR.print(os);
  return os;
};