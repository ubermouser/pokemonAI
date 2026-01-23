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

    Config() : neuralNet::Config() {}
    virtual ~Config() override {}

    virtual boost::program_options::options_description options(
        const std::string& category = "trainable neural network options",
        std::string prefix = "") override;
  };

  TrainableNeuralNet();
  TrainableNeuralNet(const Config& cfg, const FeatureVector& featureVector);
  TrainableNeuralNet(const Config& cfg, size_t inputSize, size_t outputSize);

  virtual ~TrainableNeuralNet() override;

  TrainableNeuralNet& initialize() override;

  torch::optim::Adam& getOptimizer() { return *optimizer_; }

 protected:
  Config cfg_;
  std::unique_ptr<torch::optim::Adam> optimizer_;
  void initOptimizer(const Config& cfg);
};

#endif // TRAINABLE_NEURALNET_H
