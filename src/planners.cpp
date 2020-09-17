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
  if (type == "maximin") {
    return std::make_shared<PlannerMaxiMin::Config>();
  } else if (type == "minimax") {
    return std::make_shared<PlannerMiniMax::Config>();
  } else if (type == "random") {
    return std::make_shared<PlannerRandom::Config>();
  } else if (type == "max") {
    return std::make_shared<PlannerMax::Config>();
  } else if (type == "human") {
    return std::make_shared<PlannerHuman::Config>();
  } else {
    return std::make_shared<Planner::Config>();
  }
}


std::shared_ptr<Planner> planners::choose(const std::string& _type, const Planner::Config& cfg) {
  auto type = boost::to_lower_copy(_type);
  if (type == "maximin") {
    return std::make_shared<PlannerMaxiMin>(dynamic_cast<const PlannerMaxiMin::Config&>(cfg));
  } else if (type == "minimax") {
    return std::make_shared<PlannerMiniMax>(dynamic_cast<const PlannerMiniMax::Config&>(cfg));
  } else if (type == "random") {
    return std::make_shared<PlannerRandom>(dynamic_cast<const PlannerRandom::Config&>(cfg));
  } else if (type == "max") {
    return std::make_shared<PlannerMax>(dynamic_cast<const PlannerMax::Config&>(cfg));
  } else if (type == "human") {
    return std::make_shared<PlannerHuman>(dynamic_cast<const PlannerHuman::Config&>(cfg));
  } else {
    std::cerr << "unknown planner type \"" << _type << "\"!\n";
    throw std::invalid_argument("planner type");
  }
}
