#include "../inc/planners.h"

#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <boost/algorithm/string/case_conv.hpp>

#include "../inc/orphan.h"
#include "../inc/planner_random.h"
#include "../inc/planner_human.h"
#include "../inc/planner_max.h"
#include "../inc/planner_maximin.h"
#include "../inc/planner_minimax.h"

std::shared_ptr<Planner::Config> planners::config(const std::string& _type) {
  auto type = boost::to_lower_copy(_type);
  std::shared_ptr<Planner::Config> result;
  if (type == "maximin") {
    result = std::make_shared<PlannerMaxiMin::Config>();
  } else if (type == "minimax") {
    result = std::make_shared<PlannerMiniMax::Config>();
  } else if (type == "random") {
    result = std::make_shared<PlannerRandom::Config>();
  } else if (type == "max") {
    result = std::make_shared<PlannerMax::Config>();
  } else if (type == "human") {
    result = std::make_shared<PlannerHuman::Config>();
  } else {
    result = std::make_shared<Planner::Config>();
  }
  return result;
}


std::shared_ptr<Planner> planners::choose(const std::string& _type, const Planner::Config& cfg) {
  auto type = boost::to_lower_copy(_type);
  std::shared_ptr<Planner> result;
  if (type == "maximin") {
    result = std::make_shared<PlannerMaxiMin>(dynamic_cast<const PlannerMaxiMin::Config&>(cfg));
  } else if (type == "minimax") {
    result = std::make_shared<PlannerMiniMax>(dynamic_cast<const PlannerMiniMax::Config&>(cfg));
  } else if (type == "random") {
    result = std::make_shared<PlannerRandom>(dynamic_cast<const PlannerRandom::Config&>(cfg));
  } else if (type == "max") {
    result = std::make_shared<PlannerMax>(dynamic_cast<const PlannerMax::Config&>(cfg));
  } else if (type == "human") {
    result = std::make_shared<PlannerHuman>(dynamic_cast<const PlannerHuman::Config&>(cfg));
  } else {
    std::cerr << "unknown planner type \"" << _type << "\"!\n";
    throw std::invalid_argument("planner type");
  }
  return result;
}
