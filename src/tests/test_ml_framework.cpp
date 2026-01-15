#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "pokemonai/feature_vector.h"
#include "pokemonai/neuralNet.h"

class MockFeatureVector : public FeatureVector {
 public:
  MockFeatureVector(size_t input, size_t output)
      : input_(input), output_(output) {}
  size_t inputSize() const override { return input_; }
  size_t outputSize() const override { return output_; }

  void seed(
      floatIterator_t cInput,
      const ConstEnvironmentVolatile& env,
      size_t iTeam) const override {}

 private:
  size_t input_;
  size_t output_;
};

TEST(MLFrameworkTest, BasicForwardPass) {
  // Initialize a simple network with topology [2, 4, 1]
  MockFeatureVector mfv(2, 1);
  neuralNet::Config cfg;
  cfg.architecture = {4};
  neuralNet net(cfg, mfv);

  EXPECT_TRUE(net.isInitialized());
  EXPECT_EQ(net.numInputs(), 2);
  EXPECT_EQ(net.numOutputs(), 1);

  // Set dummy input
  std::vector<float> input = {0.5f, -0.2f};
  net.feedForward(input.begin());

  // Check output
  float output = *net.outputBegin();
  EXPECT_GE(output, 0.0f);
  EXPECT_LE(output, 1.0f);
  EXPECT_FALSE(std::isnan(output));
}

TEST(MLFrameworkTest, WeightRandomization) {
  MockFeatureVector mfv(2, 2);
  neuralNet::Config cfg;
  neuralNet net(cfg, mfv);

  net.randomizeWeights();

  std::vector<float> input = {1.0f, 1.0f};
  net.feedForward(input.begin());
  float output1 = *net.outputBegin();

  net.randomizeWeights();
  net.feedForward(input.begin());
  float output2 = *net.outputBegin();

  // Highly unlikely to be exactly the same after randomization
  EXPECT_NE(output1, output2);
}

TEST(MLFrameworkTest, Serialization) {
  MockFeatureVector mfv(2, 1);
  neuralNet::Config cfg;
  cfg.architecture = {4};
  neuralNet net1(cfg, mfv);
  net1.randomizeWeights();

  // Save to stream
  std::stringstream ss;
  net1.output(ss);

  // Load into a new network with same topology
  neuralNet net2(cfg, mfv);
  EXPECT_TRUE(net2.input(ss));

  // Compare outputs
  std::vector<float> input = {0.8f, -0.5f};
  net1.feedForward(input.begin());
  net2.feedForward(input.begin());

  EXPECT_EQ(*net1.outputBegin(), *net2.outputBegin());
}
