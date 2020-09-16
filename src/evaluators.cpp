#include "../inc/evaluators.h"

#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <boost/algorithm/string/case_conv.hpp>

#include "../inc/orphan.h"
#include "../inc/evaluator_montecarlo.h"
#include "../inc/evaluator_random.h"
#include "../inc/evaluator_simple.h"

std::shared_ptr<Evaluator::Config> evaluators::config(const std::string& _type) {
  auto type = boost::to_lower_copy(_type);
  if (type == "simple") {
    return std::make_shared<EvaluatorSimple::Config>();
  } else if (type == "random") {
    return std::make_shared<EvaluatorRandom::Config>();
  } else if (type == "montecarlo") {
    return std::make_shared<EvaluatorMonteCarlo::Config>();
  } else {
    return std::make_shared<Evaluator::Config>();
  }
}


std::shared_ptr<Evaluator> evaluators::choose(const std::string& _type, const Evaluator::Config& cfg) {
  auto type = boost::to_lower_copy(_type);
  if (type == "simple") {
    return std::make_shared<EvaluatorSimple>(dynamic_cast<const EvaluatorSimple::Config&>(cfg));
  } else if (type == "random") {
    return std::make_shared<EvaluatorRandom>(dynamic_cast<const EvaluatorRandom::Config&>(cfg));
  } else if (type == "montecarlo") {
    return std::make_shared<EvaluatorMonteCarlo>(dynamic_cast<const EvaluatorMonteCarlo::Config&>(cfg));
  } else {
    std::cerr << "unknown evaluator type \"" << _type << "\"!\n";
    throw std::invalid_argument("evaluator type");
  }
}