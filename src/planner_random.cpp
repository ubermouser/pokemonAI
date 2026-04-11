//#define PKAI_IMPORT
#include "pokemonai/planner_random.h"

#include <fmt/format.h>

#include <boost/program_options.hpp>

#include "pokemonai/environment_possible.h"
#include "pokemonai/pkCU.h"

namespace po = boost::program_options;


po::options_description PlannerRandom::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc = base_t::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "move-chance").c_str(),
      po::value<double>(&moveChance)->default_value(defaults.moveChance),
      "likelihood for the random action selected to be a move.");

  return desc;
}


void PlannerRandom::resetName() {
  setName(fmt::format("{}(c={:3.1f})", baseName(), cfg_.moveChance));
}


PlyResult PlannerRandom::generateSolutionAtLeaf(
    const ConstEnvironmentPossible& origin) const {
  PlyResult result;
  auto validActions = cu_->getAllValidActions(origin.getEnv(), agentTeam_);

  // TODO: choose move actions at a higher probability than switch actions
  // are there ANY valid actions?
  if (!validActions.empty()) {
    // choose a completely random action to return:
    size_t iAction = rand() % validActions.size();
    result.agentAction = validActions[iAction];
  }

  assert(!result.agentAction.empty());
  return result;
};
