#ifndef TRAINER_REGRESS_FITNESS_H
#define TRAINER_REGRESS_FITNESS_H

#include <torch/torch.h>

#include <memory>

#include "pokemonai/feature_vector.h"
#include "pokemonai/game.h"
#include "pokemonai/league_heat.h"
#include "pokemonai/trainable_neural_net.h"

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

class TrainerRegressFitness {
public:
  struct Config {
    /* number of samples per batch during training and evaluation */
    size_t batchSize = 1024;
    /* number of batches between logging loss events. If 0, logging is disabled. */
    size_t logInterval = 1000;
    /* number of times to iterate over the entire dataset during training */
    size_t numEpochs = 1;

    /* random seed used for sampling and torch operations. If 0, a random seed is used. */
    uint64_t seed = 0;

    /* temporal-difference learning discount factor. If 1.0, no discount is
     * applied. */
    double discountFactor = 0.96;

    Config() {}

    boost::program_options::options_description options(
        const std::string& category = "trainer options",
        std::string prefix = "");
  };

  TrainerRegressFitness(
      std::shared_ptr<FeatureVector> featureVector,
      std::shared_ptr<TrainableNeuralNet> neuralNet,
      const Config& cfg = Config());

  float fit(const HeatResult& hResult);
  float fit(const LeagueHeat& lHeat);
  float predict(const HeatResult& hResult);
  float predict(const LeagueHeat& lHeat);

 protected:
  std::vector<HeatDataset::Sample> prepareDataset(const HeatResult& hResult);
  std::vector<HeatDataset::Sample> prepareDataset(const LeagueHeat& lHeat);
  float fit(std::vector<HeatDataset::Sample> samples);
  float predict(std::vector<HeatDataset::Sample> samples) const;

  std::shared_ptr<FeatureVector> featureVector_;
  std::shared_ptr<TrainableNeuralNet> neuralNet_;
  Config cfg_;
};

#endif // TRAINER_REGRESS_FITNESS_H
