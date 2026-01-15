#ifndef TRAINER_H
#define TRAINER_H

#include "pokemonai/game.h"
#include "pokemonai/feature_vector.h"
#include "pokemonai/trainable_neural_net.h"
#include <memory>
#include <torch/torch.h>

class HeatDataset : public torch::data::Dataset<HeatDataset> {
public:
  struct Sample {
    torch::Tensor input;
    torch::Tensor target;
  };

  HeatDataset(std::vector<Sample> samples) : samples_(std::move(samples)) {}

  torch::data::Example<> get(size_t index) override {
    return {samples_[index].input, samples_[index].target};
  }

  torch::optional<size_t> size() const override {
    return samples_.size();
  }

protected:
  std::vector<Sample> samples_;
};

class Trainer {
public:
  struct Config {
    /* number of samples per batch during training and evaluation */
    size_t batchSize = 32;
    /* number of batches between logging loss events. If 0, logging is disabled. */
    size_t logInterval = 10;
    /* number of times to iterate over the entire dataset during training */
    size_t numEpochs = 1;

    /* random seed used for sampling and torch operations. If 0, a random seed is used. */
    uint64_t seed = 0;

    Config() {}

    boost::program_options::options_description options(
        const std::string& category = "trainer options",
        std::string prefix = "");
  };

  Trainer(
      std::shared_ptr<FeatureVector> featureVector,
      std::shared_ptr<TrainableNeuralNet> neuralNet,
      std::shared_ptr<const EnvironmentNonvolatile> envNV,
      const Config& cfg = Config());

  float fit(const HeatResult& hResult);
  float predict(const HeatResult& hResult);

protected:
  std::vector<HeatDataset::Sample> prepareDataset(const HeatResult& hResult) const;

  std::shared_ptr<FeatureVector> featureVector_;
  std::shared_ptr<TrainableNeuralNet> neuralNet_;
  std::shared_ptr<const EnvironmentNonvolatile> envNV_;
  Config cfg_;
};

#endif // TRAINER_H
