#ifndef TRAINABLE_NEURAL_NET_H
#define TRAINABLE_NEURAL_NET_H

#include "pokemonai/neuralNet.h"
#include <torch/torch.h>
#include <memory>
#include <string>

class TrainableNeuralNet : public neuralNet {
 public:
  struct Config : public neuralNet::Config {
    double learningRate = 1e-3;
    double adamBeta1 = 0.9;
    double adamBeta2 = 0.999;
    double adamEpsilon = 1e-8;
    double weightDecay = 0;
    bool randomWeights = false;

    Config() : neuralNet::Config() {}
    virtual ~Config() override {}

    virtual boost::program_options::options_description options(
        const std::string& category = "trainable neural network options",
        std::string prefix = "") override;
  };

  TrainableNeuralNet();
  TrainableNeuralNet(const Config& cfg, const FeatureVector& featureVector);
  TrainableNeuralNet(const Config& cfg, size_t inputSize, size_t outputSize);
  TrainableNeuralNet(const TrainableNeuralNet& other);

  virtual ~TrainableNeuralNet() override;

  TrainableNeuralNet& initialize() override;

  virtual std::shared_ptr<neuralNet> clone() const override {
    return std::make_shared<TrainableNeuralNet>(*this);
  }

  /* just as it says, randomizes ALL the weights of this neural network */
  void randomizeWeights();

  /* jitters the network's weight */
  void jitterWeights(float jitterMax);

  torch::optim::Adam& getOptimizer() { return *optimizer_; }

 protected:
  Config cfg_;
  std::unique_ptr<torch::optim::Adam> optimizer_;
  void initOptimizer(const Config& cfg);
};

#endif // TRAINABLE_NEURALNET_H
