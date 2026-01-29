#include "pokemonai/planners.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "pokemonai/orphan.h"
#include "pokemonai/planner_human.h"
#include "pokemonai/planner_max.h"
#include "pokemonai/planner_maximin.h"
#include "pokemonai/planner_minimax.h"
#include "pokemonai/planner_negamax.h"
#include "pokemonai/planner_random.h"
#include "pokemonai/planner_softmax.h"

std::shared_ptr<Planner::Config> planners::config(const std::string& _type) {
  auto type = boost::to_lower_copy(_type);
  std::shared_ptr<Planner::Config> result;
  if (type == "maximin") {
    result = std::make_shared<PlannerMaximin::Config>();
  } else if (type == "minimax") {
    result = std::make_shared<PlannerMinimax::Config>();
  } else if (type == "negamax") {
    result = std::make_shared<PlannerNegamax::Config>();
  } else if (type == "random") {
    result = std::make_shared<PlannerRandom::Config>();
  } else if (type == "max") {
    result = std::make_shared<PlannerMax::Config>();
  } else if (type == "human") {
    result = std::make_shared<PlannerHuman::Config>();
  } else if (type == "softmax") {
    result = std::make_shared<PlannerSoftmax::Config>();
  } else {
    result = std::make_shared<Planner::Config>();
  }
  return result;
}


std::shared_ptr<Planner> planners::choose(const std::string& _type, const Planner::Config& cfg) {
  auto type = boost::to_lower_copy(_type);
  std::shared_ptr<Planner> result;
  if (type == "maximin") {
    result = std::make_shared<PlannerMaximin>(
        dynamic_cast<const PlannerMaximin::Config&>(cfg));
  } else if (type == "minimax") {
    result = std::make_shared<PlannerMinimax>(
        dynamic_cast<const PlannerMinimax::Config&>(cfg));
  } else if (type == "negamax") {
    result = std::make_shared<PlannerNegamax>(
        dynamic_cast<const PlannerNegamax::Config&>(cfg));
  } else if (type == "random") {
    result = std::make_shared<PlannerRandom>(
        dynamic_cast<const PlannerRandom::Config&>(cfg));
  } else if (type == "max") {
    result = std::make_shared<PlannerMax>(
        dynamic_cast<const PlannerMax::Config&>(cfg));
  } else if (type == "human") {
    result = std::make_shared<PlannerHuman>(
        dynamic_cast<const PlannerHuman::Config&>(cfg));
  } else if (type == "softmax") {
    result = std::make_shared<PlannerSoftmax>(
        dynamic_cast<const PlannerSoftmax::Config&>(cfg));
  } else {
    SPDLOG_ERROR("unknown planner type \"{}\"!", _type);
    throw std::invalid_argument("planner type");
  }
  return result;
}
