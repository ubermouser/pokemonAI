#include "pokemonai/evaluators.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "pokemonai/evaluator_montecarlo.h"
#include "pokemonai/evaluator_network128.h"
#include "pokemonai/evaluator_network16.h"
#include "pokemonai/evaluator_network32.h"
#include "pokemonai/evaluator_network64.h"
#include "pokemonai/evaluator_network_large.h"
#include "pokemonai/evaluator_random.h"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/orphan.h"

std::shared_ptr<Evaluator::Config> evaluators::config(const std::string& _type) {
  auto type = boost::to_lower_copy(_type);
  std::shared_ptr<Evaluator::Config> result;
  if (type == "simple") {
    result = std::make_shared<EvaluatorSimple::Config>();
  } else if (type == "random") {
    result = std::make_shared<EvaluatorRandom::Config>();
  } else if (type == "montecarlo") {
    result = std::make_shared<EvaluatorMonteCarlo::Config>();
  } else if (
      type == "network16" || type == "network32" || type == "network64" ||
      type == "network128") {
    result = std::make_shared<EvaluatorNetwork::Config>();
  } else if (
      type == "tnetwork16" || type == "tnetwork32" || type == "tnetwork64" ||
      type == "tnetwork128") {
    result = std::make_shared<TrainableEvaluatorNetwork::Config>();
  } else if (type == "networklarge") {
    result = std::make_shared<EvaluatorNetworkLarge::Config>();
  } else if (type == "tnetworklarge") {
    result = std::make_shared<TrainableEvaluatorNetworkLarge::Config>();
  } else {
    result = std::make_shared<Evaluator::Config>();
  }
  return result;
}


std::shared_ptr<Evaluator> evaluators::choose(const std::string& _type, const Evaluator::Config& cfg) {
  auto type = boost::to_lower_copy(_type);
  std::shared_ptr<Evaluator> result;
  if (type == "simple") {
    result = std::make_shared<EvaluatorSimple>(
        dynamic_cast<const EvaluatorSimple::Config&>(cfg));
  } else if (type == "random") {
    result = std::make_shared<EvaluatorRandom>(
        dynamic_cast<const EvaluatorRandom::Config&>(cfg));
  } else if (type == "montecarlo") {
    result = std::make_shared<EvaluatorMonteCarlo>(
        dynamic_cast<const EvaluatorMonteCarlo::Config&>(cfg));
  } else if (type == "network16") {
    result = std::make_shared<evaluator_network16>(
        dynamic_cast<const EvaluatorNetwork::Config&>(cfg));
  } else if (type == "tnetwork16") {
    result = std::make_shared<trainable_evaluator_network16>(
        dynamic_cast<const TrainableEvaluatorNetwork::Config&>(cfg));
  } else if (type == "network32") {
    result = std::make_shared<evaluator_network32>(
        dynamic_cast<const EvaluatorNetwork::Config&>(cfg));
  } else if (type == "tnetwork32") {
    result = std::make_shared<trainable_evaluator_network32>(
        dynamic_cast<const TrainableEvaluatorNetwork::Config&>(cfg));
  } else if (type == "network64") {
    result = std::make_shared<evaluator_network64>(
        dynamic_cast<const EvaluatorNetwork::Config&>(cfg));
  } else if (type == "tnetwork64") {
    result = std::make_shared<trainable_evaluator_network64>(
        dynamic_cast<const TrainableEvaluatorNetwork::Config&>(cfg));
  } else if (type == "network128") {
    result = std::make_shared<evaluator_network128>(
        dynamic_cast<const EvaluatorNetwork::Config&>(cfg));
  } else if (type == "tnetwork128") {
    result = std::make_shared<trainable_evaluator_network128>(
        dynamic_cast<const TrainableEvaluatorNetwork::Config&>(cfg));
  } else if (type == "networklarge") {
    result = std::make_shared<EvaluatorNetworkLarge>(
        dynamic_cast<const EvaluatorNetworkLarge::Config&>(cfg));
  } else if (type == "tnetworklarge") {
    result = std::make_shared<TrainableEvaluatorNetworkLarge>(
        dynamic_cast<const TrainableEvaluatorNetworkLarge::Config&>(cfg));
  } else {
    SPDLOG_ERROR("unknown evaluator type \"{}\"!", _type);
    throw std::invalid_argument("evaluator type");
  }
  return result;
}