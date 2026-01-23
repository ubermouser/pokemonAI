#include "pokemonai/trainer.h"

#include <spdlog/spdlog.h>

#include <fstream>
#include <unordered_set>

#include "pokemonai/evaluator_network.h"

namespace po = boost::program_options;

po::options_description Trainer::Config::options(
    const std::string& category, std::string prefix) {
  auto desc = TeamBuilder::Config::options(category, prefix);
  desc.add(training.options(category + " [Training]", prefix + "training"));
  return desc;
}

Trainer::Trainer(const Config& cfg) : TeamBuilder(cfg), cfg_(cfg) {}

void Trainer::initialize() {
  TeamBuilder::initialize();

  std::unordered_set<std::shared_ptr<TrainableNeuralNet>> uniqueNetworks;
  trainableNetworks_.clear();

  for (auto& pair : initialLeague_.evaluators) {
    auto evalPtr =
        std::dynamic_pointer_cast<EvaluatorNetwork>(pair.second->getPtr());
    if (!evalPtr) { continue; }

    auto net =
        std::dynamic_pointer_cast<TrainableNeuralNet>(evalPtr->getNetwork());
    if (!net) { continue; }

    if (uniqueNetworks.find(net) == uniqueNetworks.end()) {
      net->initialize();
      auto trainer = std::make_shared<TrainerRegressFitness>(
          evalPtr, net, cfg_.training);
      trainableNetworks_.push_back({evalPtr, net, trainer});
      uniqueNetworks.insert(net);
    }
  }
}

void Trainer::postGenerationHook(LeagueHeat& league) const {
  // After every heat, train the networks:
  train(league);
}

void Trainer::postEvolveHook(LeagueHeat& league) const {
  if (cfg_.verbosity >= 1) { out_.get() << "Training Complete!\n"; }
  if (cfg_.verbosity >= 2) { printTrainingResults(league); }
  // Final save of trained networks
  if (cfg_.saveOnCompletion) { saveTrainedNetworks(); }
}

void Trainer::train(LeagueHeat& league) const {
  for (auto& tp : trainableNetworks_) {
    SPDLOG_INFO("Training network: {}", tp.network->getName());
    tp.trainer->fit(league);
  }
}

void Trainer::saveTrainedNetworks() const {
  for (auto& tp : trainableNetworks_) {
    const auto& netCfg = tp.evaluator->getConfig().netConfig;
    if (!netCfg.modelPath.empty()) {
      SPDLOG_INFO(
          "Saving trained network {} to {}",
          tp.network->getName(),
          netCfg.modelPath);
      std::ofstream oFile(netCfg.modelPath, std::ios::binary);
      if (oFile) {
        tp.network->output(oFile);
      } else {
        SPDLOG_ERROR(
            "Failed to open model path {} for writing", netCfg.modelPath);
      }
    } else {
      SPDLOG_WARN(
          "Trained network {} has no model path defined, not saving "
          "to disk",
          tp.network->getName());
    }
  }
}

void Trainer::printTrainingResults(const LeagueHeat& league) const {
  out_.get() << "---- TRAINING RESULTS (Final Loss on Last Heat) ----\n";

  for (auto& tp : trainableNetworks_) {
    float loss = tp.trainer->predict(league);
    std::string name = tp.evaluator->getName();
    out_.get() << fmt::format(" {}: loss={:.6f}\n", name, loss);
  }
}
