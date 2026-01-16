#include "pokemonai/trainer_regress_fitness.h"
#include "pokemonai/environment_possible.h"
#include <torch/torch.h>
#include <vector>
#include <spdlog/spdlog.h>

namespace po = boost::program_options;


boost::program_options::options_description TrainerRegressFitness::Config::options(
    const std::string& category, std::string prefix) {
  po::options_description desc(category);
  if (prefix.size() > 0 && prefix.back() != '-') { prefix.append("-"); }
  // clang-format off
  desc.add_options()
    ((prefix + "batch-size").c_str(), 
    po::value<size_t>(&batchSize)->default_value(batchSize), 
    "Size of training batches")
    ((prefix + "log-interval").c_str(),
    po::value<size_t>(&logInterval)->default_value(logInterval),
    "Number of batches between logging loss")
    ((prefix + "epochs").c_str(),
    po::value<size_t>(&numEpochs)->default_value(numEpochs),
    "Number of training epochs")
    ((prefix + "seed").c_str(),
    po::value<uint64_t>(&seed)->default_value(seed),
    "Random seed for training. If 0, a random seed is used.");
  // clang-format on
  return desc;
}


TrainerRegressFitness::TrainerRegressFitness(
    std::shared_ptr<FeatureVector> featureVector,
    std::shared_ptr<TrainableNeuralNet> neuralNet,
    const Config& cfg)
    : featureVector_(featureVector), neuralNet_(neuralNet), cfg_(cfg) {}


float TrainerRegressFitness::fit(const HeatResult& hResult) {
  return fit(prepareDataset(hResult));
}


float TrainerRegressFitness::fit(const LeagueHeat& lHeat) {
  return fit(prepareDataset(lHeat));
}


float TrainerRegressFitness::fit(std::vector<HeatDataset::Sample> samples) {
  if (cfg_.seed != 0) {
    torch::manual_seed(cfg_.seed);
  }

  if (samples.empty()) return 0.0f;

  // 2. Create DataLoader
  auto dataset = HeatDataset(std::move(samples)).map(torch::data::transforms::Stack<>());
  auto dataLoader = torch::data::make_data_loader<torch::data::samplers::RandomSampler>(
      std::move(dataset),
      torch::data::DataLoaderOptions().batch_size(cfg_.batchSize));

  // 3. Training Loop
  float totalLoss = 0;
  size_t totalBatchCount = 0;
  auto& model = neuralNet_->getModel();
  auto& optimizer = neuralNet_->getOptimizer();

  model->train();
  for (size_t iEpoch = 0; iEpoch < cfg_.numEpochs; ++iEpoch) {
    float epochLoss = 0;
    size_t epochBatchCount = 0;

    for (auto& batch : *dataLoader) {
      optimizer.zero_grad();
      
      auto output = model->forward(batch.data);
      auto loss = torch::mse_loss(output, batch.target);
      
      loss.backward();
      optimizer.step();

      float currentLoss = loss.item<float>();
      epochLoss += currentLoss;
      epochBatchCount++;
      totalBatchCount++;

      if (cfg_.logInterval > 0 && (totalBatchCount % cfg_.logInterval == 0)) {
        SPDLOG_INFO("Epoch {}/{} Batch {}: Loss = {:.6f}", iEpoch + 1, cfg_.numEpochs, totalBatchCount, currentLoss);
      }
    }
    totalLoss += epochLoss / (epochBatchCount > 0 ? epochBatchCount : 1);
  }

  float finalLoss = cfg_.numEpochs > 0 ? totalLoss / cfg_.numEpochs : 0.0f;
  SPDLOG_INFO("Training completed with average loss: {:.6f}", finalLoss);
  return finalLoss;
}


float TrainerRegressFitness::predict(const HeatResult& hResult) {
  return predict(prepareDataset(hResult));
}


float TrainerRegressFitness::predict(const LeagueHeat& lHeat) {
  return predict(prepareDataset(lHeat));
}


float TrainerRegressFitness::predict(std::vector<HeatDataset::Sample> samples) const {
  if (samples.empty()) return 0.0f;

  auto dataset = HeatDataset(std::move(samples)).map(torch::data::transforms::Stack<>());
  auto dataLoader = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(
      std::move(dataset),
      torch::data::DataLoaderOptions().batch_size(cfg_.batchSize));

  float totalLoss = 0;
  size_t totalBatchCount = 0;
  auto& model = neuralNet_->getModel();

  torch::NoGradGuard no_grad;
  model->eval();
  for (auto& batch : *dataLoader) {
    auto output = model->forward(batch.data);
    auto loss = torch::mse_loss(output, batch.target);

    totalLoss += loss.item<float>();
    totalBatchCount++;
  }

  float finalLoss = totalBatchCount > 0 ? totalLoss / totalBatchCount : 0.0f;
  SPDLOG_INFO("Evaluation completed with average loss: {:.6f}", finalLoss);
  return finalLoss;
}


std::vector<HeatDataset::Sample> TrainerRegressFitness::prepareDataset(
    const HeatResult& hResult) {
  featureVector_->setEnvironment(hResult.nv);

  std::vector<HeatDataset::Sample> samples;
  for (const auto& gResult : hResult.gameResults) {
    for (const auto& turn : gResult.log) {
      ConstEnvironmentVolatile cev(*hResult.nv, turn.env.env);
      for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
        std::vector<float> inputData(featureVector_->inputSize());
        featureVector_->seed(inputData.begin(), cev, iTeam);

        auto inputTensor = torch::from_blob(inputData.data(), {(long)inputData.size()}, torch::kFloat).clone();
        auto targetTensor = torch::tensor({(float)turn.teams[iTeam].simpleFitness}, torch::kFloat);

        samples.push_back({inputTensor, targetTensor});
      }
    }
  }

  SPDLOG_DEBUG("Extracted {} samples from {} matches", samples.size(), hResult.matchesPlayed);
  return samples;
}


std::vector<HeatDataset::Sample> TrainerRegressFitness::prepareDataset(
    const LeagueHeat& lHeat) {
  std::vector<HeatDataset::Sample> allSamples;
  for (const auto& game : lHeat.games) {
    auto samples = prepareDataset(game.heatResult);
    allSamples.insert(
        allSamples.end(),
        std::make_move_iterator(samples.begin()),
        std::make_move_iterator(samples.end()));
  }

  SPDLOG_INFO(
      "Prepared dataset with {} samples from {} games",
      allSamples.size(),
      lHeat.games.size());
  return allSamples;
}
