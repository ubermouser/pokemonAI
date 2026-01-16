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

LeagueHeat Trainer::evolve() const {
  testInitialized();

  LeagueHeat league = constructLeague();
  for (size_t iGeneration = 0; iGeneration < cfg_.maxGenerations;
       ++iGeneration) {
    if (iGeneration > 0) {
      // perform an evolution step (if this is not the first generation):
      evolveGeneration(league);

      // reset league counting:
      resetLeague(league);
    }

    if (cfg_.verbosity >= 1) { printGenerationStart(league, iGeneration); }

    // rank the league:
    runLeague(league);

    // After every heat, train the networks:
    train(league);
  }

  if (cfg_.verbosity >= 1) {
    out_.get() << "Evolution and Training Complete!\n";
  }
  if (cfg_.verbosity >= 2) { printLeagueCounts(league); }
  if (cfg_.saveOnCompletion) { saveTeamPopulation(league); }

  // Final save of trained networks
  saveTrainedNetworks(league);

  return league;
}

void Trainer::train(LeagueHeat& league) const {
  std::unordered_set<std::shared_ptr<TrainableNeuralNet>> trainedNetworks;

  for (auto& pair : league.evaluators) {
    auto evalPtr = pair.second->getPtr();
    if (auto evalNet = std::dynamic_pointer_cast<EvaluatorNetwork>(evalPtr)) {
      auto net = evalNet->getNetwork();
      if (auto trainable = std::dynamic_pointer_cast<TrainableNeuralNet>(net)) {
        if (trainedNetworks.find(trainable) == trainedNetworks.end()) {
          SPDLOG_INFO("Training network: {}", trainable->getName());
          TrainerRegressFitness trainer(evalNet, trainable, cfg_.training);
          trainer.fit(league);
          trainedNetworks.insert(trainable);
        }
      }
    }
  }
}

void Trainer::saveTrainedNetworks(const LeagueHeat& league) const {
  std::unordered_set<std::shared_ptr<TrainableNeuralNet>> savedNetworks;

  for (auto& pair : league.evaluators) {
    auto evalPtr = pair.second->getPtr();
    if (auto evalNet = std::dynamic_pointer_cast<EvaluatorNetwork>(evalPtr)) {
      auto net = evalNet->getNetwork();
      if (auto trainable = std::dynamic_pointer_cast<TrainableNeuralNet>(net)) {
        if (savedNetworks.find(trainable) == savedNetworks.end()) {
          const auto& netCfg = evalNet->getConfig().netConfig;
          if (!netCfg.checkpointPath.empty()) {
            SPDLOG_INFO(
                "Saving trained network {} to {}",
                trainable->getName(),
                netCfg.checkpointPath);
            std::ofstream oFile(netCfg.checkpointPath, std::ios::binary);
            if (oFile) {
              trainable->output(oFile);
            } else {
              SPDLOG_ERROR(
                  "Failed to open checkpoint path {} for writing",
                  netCfg.checkpointPath);
            }
          } else {
            SPDLOG_WARN(
                "Trained network {} has no checkpoint path defined, not saving "
                "to disk",
                trainable->getName());
          }
          savedNetworks.insert(trainable);
        }
      }
    }
  }
}
