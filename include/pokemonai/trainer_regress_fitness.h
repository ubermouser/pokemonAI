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

    /**
     * Maximum number of training examples to keep in the buffer.
     * Earliest examples are truncated when the buffer exceeds this size.
     */
    size_t datasetSize = 0;

    Config() {}

    boost::program_options::options_description options(
        const std::string& category = "trainer options",
        std::string prefix = "");
  };

  TrainerRegressFitness(
      std::shared_ptr<FeatureVector> featureVector,
      std::shared_ptr<TrainableNeuralNet> neuralNet,
      const Config& cfg = Config());

  /**
   * Translates a single HeatResult into training samples and adds them to the
   * buffer.
   * @return The updated number of samples in the buffer.
   */
  size_t addTrainingData(const HeatResult& hResult);

  /**
   * Translates an entire LeagueHeat into training samples and adds them to the
   * buffer.
   * @return The updated number of samples in the buffer.
   */
  size_t addTrainingData(const LeagueHeat& lHeat);

  /**
   * Adds the given HeatResult to the buffer and trains the model on the entire
   * contents of the buffer.
   * @return The average training loss.
   */
  float fit(const HeatResult& hResult);

  /**
   * Adds the given LeagueHeat to the buffer and trains the model on the entire
   * contents of the buffer.
   * @return The average training loss.
   */
  float fit(const LeagueHeat& lHeat);

  /**
   * Predicts the loss for the given HeatResult without training.
   * @return The average loss.
   */
  float predict(const HeatResult& hResult);

  /**
   * Predicts the loss for the given LeagueHeat without training.
   * @return The average loss.
   */
  float predict(const LeagueHeat& lHeat);

 protected:
  std::vector<HeatDataset::Sample> prepareDataset(const HeatResult& hResult);
  std::vector<HeatDataset::Sample> prepareDataset(const LeagueHeat& lHeat);

  /**
   * Trains the model on the current contents of the buffer.
   */
  float fit();
  float predict(std::vector<HeatDataset::Sample> samples) const;
  size_t addTrainingData(std::vector<HeatDataset::Sample> samples);

  std::shared_ptr<FeatureVector> featureVector_;
  std::shared_ptr<TrainableNeuralNet> neuralNet_;
  Config cfg_;

  std::vector<HeatDataset::Sample> buffer_;
};

#endif // TRAINER_REGRESS_FITNESS_H
