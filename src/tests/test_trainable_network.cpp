#include <gtest/gtest.h>
#include "pokemonai/trainable_neural_net.h"
#include "pokemonai/feature_vector.h"

class MockFeatureVector : public FeatureVector {
 public:
  size_t inputSize() const override { return 10; }
  size_t outputSize() const override { return 2; }
};

TEST(TrainableNeuralNetTest, Initialization) {
  TrainableNeuralNet::Config cfg;
  cfg.learningRate = 0.01;
  cfg.architecture = {5};
  
  MockFeatureVector fv;
  TrainableNeuralNet net(cfg, fv);
  
  EXPECT_TRUE(net.isInitialized());
  EXPECT_EQ(net.numInputs(), 10);
  EXPECT_EQ(net.numOutputs(), 2);
  
  // Verify optimizer is initialized
  EXPECT_NO_THROW(net.getOptimizer());
}

TEST(TrainableNeuralNetTest, OptimizerSettings) {
  TrainableNeuralNet::Config cfg;
  cfg.learningRate = 0.005;
  cfg.adamBeta1 = 0.8;
  cfg.adamBeta2 = 0.99;
  cfg.adamEpsilon = 1e-7;
  cfg.weightDecay = 1e-4;
  
  TrainableNeuralNet net(cfg, 10, 2);
  
  auto& optimizer = net.getOptimizer();
  auto& options = static_cast<torch::optim::AdamOptions&>(optimizer.defaults());
  
  EXPECT_DOUBLE_EQ(options.lr(), 0.005);
  EXPECT_DOUBLE_EQ(std::get<0>(options.betas()), 0.8);
  EXPECT_DOUBLE_EQ(std::get<1>(options.betas()), 0.99);
  EXPECT_DOUBLE_EQ(options.eps(), 1e-7);
  EXPECT_DOUBLE_EQ(options.weight_decay(), 1e-4);
}
