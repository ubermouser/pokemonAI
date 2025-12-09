#include "pokemonai/evaluators.h"

#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <boost/algorithm/string/case_conv.hpp>

#include "pokemonai/orphan.h"
#include "pokemonai/evaluator_montecarlo.h"
#include "pokemonai/evaluator_random.h"
#include "pokemonai/evaluator_simple.h"

std::shared_ptr<Evaluator::Config> evaluators::config(const std::string& _type) {
  auto type = boost::to_lower_copy(_type);
  std::shared_ptr<Evaluator::Config> result;
  if (type == "simple") {
    result = std::make_shared<EvaluatorSimple::Config>();
  } else if (type == "random") {
    result = std::make_shared<EvaluatorRandom::Config>();
  } else if (type == "montecarlo") {
    result = std::make_shared<EvaluatorMonteCarlo::Config>();
  } else {
    result = std::make_shared<Evaluator::Config>();
  }
  return result;
}


std::shared_ptr<Evaluator> evaluators::choose(const std::string& _type, const Evaluator::Config& cfg) {
  auto type = boost::to_lower_copy(_type);
  std::shared_ptr<Evaluator> result;
  if (type == "simple") {
    result = std::make_shared<EvaluatorSimple>(dynamic_cast<const EvaluatorSimple::Config&>(cfg));
  } else if (type == "random") {
    result = std::make_shared<EvaluatorRandom>(dynamic_cast<const EvaluatorRandom::Config&>(cfg));
  } else if (type == "montecarlo") {
    result = std::make_shared<EvaluatorMonteCarlo>(dynamic_cast<const EvaluatorMonteCarlo::Config&>(cfg));
  } else {
    std::cerr << "unknown evaluator type \"" << _type << "\"!\n";
    throw std::invalid_argument("evaluator type");
  }
  return result;
}