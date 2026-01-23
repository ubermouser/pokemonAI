#include "pokemonai/trainable_neural_net.h"
#include <torch/torch.h>

namespace po = boost::program_options;

boost::program_options::options_description TrainableNeuralNet::Config::options(
    const std::string& category, std::string prefix) {
  auto desc = neuralNet::Config::options(category, prefix);

  if (prefix.size() > 0 && prefix.back() != '-') { prefix.append("-"); }

  // clang-format off
  desc.add_options()
    ((prefix + "learning-rate").c_str(),
     po::value<double>(&learningRate)->default_value(learningRate),
     "Learning rate for the Adam optimizer")
    ((prefix + "adam-beta1").c_str(),
     po::value<double>(&adamBeta1)->default_value(adamBeta1),
     "Beta1 hyperparameter for the Adam optimizer")
    ((prefix + "adam-beta2").c_str(),
     po::value<double>(&adamBeta2)->default_value(adamBeta2),
     "Beta2 hyperparameter for the Adam optimizer")
    ((prefix + "adam-epsilon").c_str(),
     po::value<double>(&adamEpsilon)->default_value(adamEpsilon),
     "Epsilon hyperparameter for the Adam optimizer")
    ((prefix + "weight-decay").c_str(),
     po::value<double>(&weightDecay)->default_value(weightDecay),
     "Weight decay for the Adam optimizer");
  // clang-format on
  return desc;
}


TrainableNeuralNet::TrainableNeuralNet() : neuralNet(), cfg_() {}


TrainableNeuralNet::TrainableNeuralNet(
    const Config& cfg, const FeatureVector& featureVector)
    : neuralNet(cfg, featureVector), cfg_(cfg) {}


TrainableNeuralNet::TrainableNeuralNet(
    const Config& cfg, size_t inputSize, size_t outputSize)
    : neuralNet(cfg, inputSize, outputSize), cfg_(cfg) {}


TrainableNeuralNet::~TrainableNeuralNet() {}


TrainableNeuralNet& TrainableNeuralNet::initialize() {
  neuralNet::initialize();

  if (cfg_.learningRate <= 0) {
    throw std::invalid_argument("TrainableNeuralNet: learningRate must be > 0");
  }
  if (cfg_.adamBeta1 < 0 || cfg_.adamBeta1 >= 1) {
    throw std::invalid_argument(
        "TrainableNeuralNet: adamBeta1 must be in [0, 1)");
  }
  if (cfg_.adamBeta2 < 0 || cfg_.adamBeta2 >= 1) {
    throw std::invalid_argument(
        "TrainableNeuralNet: adamBeta2 must be in [0, 1)");
  }
  if (cfg_.adamEpsilon <= 0) {
    throw std::invalid_argument("TrainableNeuralNet: adamEpsilon must be > 0");
  }
  if (cfg_.weightDecay < 0) {
    throw std::invalid_argument("TrainableNeuralNet: weightDecay must be >= 0");
  }

  initOptimizer(cfg_);

  return *this;
}


void TrainableNeuralNet::initOptimizer(const Config& cfg) {
  if (!model.is_empty() and optimizer_ == nullptr) {
    auto options = torch::optim::AdamOptions(cfg.learningRate)
                       .betas({cfg.adamBeta1, cfg.adamBeta2})
                       .eps(cfg.adamEpsilon)
                       .weight_decay(cfg.weightDecay);
    optimizer_ = std::make_unique<torch::optim::Adam>(model->parameters(), options);
  }
}
