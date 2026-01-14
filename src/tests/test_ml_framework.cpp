#include <gtest/gtest.h>
#include "pokemonai/neuralNet.h"
#include <vector>
#include <sstream>
#include <iostream>

TEST(MLFrameworkTest, BasicForwardPass) {
    // Initialize a simple network with widths [2, 4, 1]
    std::vector<size_t> widths = {2, 4, 1};
    neuralNet net(widths.begin(), widths.end());
    
    EXPECT_TRUE(net.isInitialized());
    EXPECT_EQ(net.numInputs(), 2);
    EXPECT_EQ(net.numOutputs(), 1);
    
    // Set dummy input
    std::vector<float> input = {0.5f, -0.2f};
    net.feedForward(input.begin());
    
    // Check output
    float output = net.result(0);
    EXPECT_GE(output, 0.0f);
    EXPECT_LE(output, 1.0f);
    EXPECT_FALSE(std::isnan(output));
}

TEST(MLFrameworkTest, WeightRandomization) {
    std::vector<size_t> widths = {2, 2};
    neuralNet net(widths.begin(), widths.end());
    
    net.randomizeWeights();
    
    std::vector<float> input = {1.0f, 1.0f};
    net.feedForward(input.begin());
    float output1 = net.result(0);
    
    net.randomizeWeights();
    net.feedForward(input.begin());
    float output2 = net.result(0);
    
    // Highly unlikely to be exactly the same after randomization
    EXPECT_NE(output1, output2);
}

TEST(MLFrameworkTest, Serialization) {
    std::vector<size_t> widths = {2, 4, 1};
    neuralNet net1(widths.begin(), widths.end());
    net1.randomizeWeights();
    
    // Save to stream
    std::stringstream ss;
    net1.output(ss);
    
    // Load into a new network with same topology
    neuralNet net2(widths.begin(), widths.end());
    EXPECT_TRUE(net2.input(ss));
    
    // Compare outputs
    std::vector<float> input = {0.8f, -0.5f};
    net1.feedForward(input.begin());
    net2.feedForward(input.begin());
    
    EXPECT_EQ(net1.result(0), net2.result(0));
}
