#include "../inc/true_skill.h"

#include <vector>
#include <array>
#include <boost/math/distributions/normal.hpp>
#include <boost/property_tree/ptree.hpp>
#include <ostream>
#include <iomanip>
#include <math.h>

#include "../inc/ranked.h"
#include "../inc/ranked_team.h"
#include "../inc/game.h"
#include "../inc/fp_compare.h"


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
