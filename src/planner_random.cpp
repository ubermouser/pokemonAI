//#define PKAI_IMPORT
#include "../inc/planner_random.h"

#include <typeinfo>
#include <boost/program_options.hpp>

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"

namespace po = boost::program_options;
const std::string PlannerRandom::ident = "Random_Planner-NULLEVAL";


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


PlyResult PlannerRandom::generateSolutionAtLeaf(
    const ConstEnvironmentPossible& origin) const {
  PlyResult result;
  // determine if we want to perform moves only:
  bool doMove = (rand() % RAND_MAX) <= (cfg_.moveChance*RAND_MAX);
  // determine the set of all valid actions:
  auto validMoves = cu_->getValidMoveActions(origin, agentTeam_);
  auto validActions = cu_->getValidActions(origin, agentTeam_);
  auto& valid = (doMove && !validMoves.empty())?validMoves:validActions;

  // are there ANY valid actions?
  if (!valid.empty()) {
    // choose a completely random action to return:
    size_t iAction = rand() % valid.size();
    result.agentAction = valid[iAction];
  }

  return result;
};
