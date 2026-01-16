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
      trainableNetworks_.push_back({evalPtr, net});
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
  saveTrainedNetworks();
}

void Trainer::train(LeagueHeat& league) const {
  for (auto& tp : trainableNetworks_) {
    SPDLOG_INFO("Training network: {}", tp.network->getName());
    TrainerRegressFitness trainer(tp.evaluator, tp.network, cfg_.training);
    trainer.fit(league);
  }
}

void Trainer::saveTrainedNetworks() const {
  for (auto& tp : trainableNetworks_) {
    const auto& netCfg = tp.evaluator->getConfig().netConfig;
    if (!netCfg.checkpointPath.empty()) {
      SPDLOG_INFO(
          "Saving trained network {} to {}",
          tp.network->getName(),
          netCfg.checkpointPath);
      std::ofstream oFile(netCfg.checkpointPath, std::ios::binary);
      if (oFile) {
        tp.network->output(oFile);
      } else {
        SPDLOG_ERROR(
            "Failed to open checkpoint path {} for writing",
            netCfg.checkpointPath);
      }
    } else {
      SPDLOG_WARN(
          "Trained network {} has no checkpoint path defined, not saving "
          "to disk",
          tp.network->getName());
    }
  }
}

void Trainer::printTrainingResults(const LeagueHeat& league) const {
  out_.get() << "---- TRAINING RESULTS (Final Loss on Last Heat) ----\n";

  for (auto& tp : trainableNetworks_) {
    TrainerRegressFitness trainer(tp.evaluator, tp.network, cfg_.training);
    float loss = trainer.predict(league);
    std::string name = tp.evaluator->getName();
    out_.get() << fmt::format(" {}: loss={:.6f}\n", name, loss);
  }
}
