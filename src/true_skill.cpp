#include "../inc/true_skill.h"

#include <boost/format.hpp>


namespace pt = boost::property_tree;


pt::ptree TrueSkill::output() const {
  pt::ptree result;
  result.put("mean", mean);
  result.put("stddev", stdDev);

  return result;
};


void TrueSkill::input(const pt::ptree& tree) {
  mean = tree.get<double>("mean");
  stdDev = tree.get<double>("stddev");
};


std::ostream& TrueSkill::print(std::ostream& os) const {
  os << boost::format("m=%5.2f s=%5.2f") % mean % stdDev;
  return os;
}


std::ostream& operator <<(std::ostream& os, const TrueSkill& tR) {
  tR.print(os);
  return os;
};