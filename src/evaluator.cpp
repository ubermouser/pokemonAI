//#define PKAI_IMPORT
#include "../inc/evaluator.h"

#include <stdexcept>
#include <boost/program_options.hpp>

#include "../inc/fitness.h"
#include "../inc/fp_compare.h"

namespace po = boost::program_options;


po::options_description Evaluator::Config::options(const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "team-bias").c_str(),
      po::value<fpType>(&teamBias)->default_value(defaults.teamBias),
      "home team vs away team bias.");

  return desc;

}


Evaluator& Evaluator::initialize() {
  if (nv_ == NULL) { throw std::invalid_argument("evaluator nonvolatile environment undefined"); }

  return *this;
}


Fitness Evaluator::combineTeamFitness(fpType _agentFitness, fpType _otherFitness) const {
  // calculate fitness
  fpType agentFitness = (cfg_.teamBias)       * _agentFitness;
  fpType otherFitness = (1.0 - cfg_.teamBias) * _otherFitness;

  fpType maxFitness = std::max(agentFitness, otherFitness);
  // if maxFitness is about 0, we've tied the game. Ties do not favor either team
  if (mostlyEQ(maxFitness, 0.0)) { return Fitness{0.5}; }

  fpType fitness = (agentFitness - otherFitness) / maxFitness;

  // normalize fitness from 0..1 instead of -1..1
  fitness = (fitness + 1.0) / 2.0;
  return Fitness{fitness};
}
