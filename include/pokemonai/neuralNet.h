#ifndef NEURALNET_H
#define NEURALNET_H

#include <torch/torch.h>

#include <boost/program_options.hpp>
#include <memory>
#include <vector>

#include "pokemonai/feature_vector.h"
#include "pokemonai/name.h"
#include "pokemonai/pkai.h"

class FeatureVector;


class neuralNet: public Name
{
 public:
  struct Config {
    /* Architecture of the neural network (size of hidden layers) */
    std::vector<int> architecture;

    Config() {}
    virtual ~Config() {}

    virtual boost::program_options::options_description options(
        const std::string& category = "neural network options",
        std::string prefix = "");
  };

 protected:
  torch::nn::Sequential model;
  
  // Buffers for backward compatibility with iterators
  mutable std::vector<float> inputBuffer;
  mutable std::vector<float> outputBuffer;

  std::vector<size_t> layerWidths;

 public:
  /* generates an empty invalid neural network */
  neuralNet() : model() {};

  /* creates a neural network from a config */
  neuralNet(const Config& cfg, const FeatureVector& featureVector);

  /* creates a neural network from a config with explicit sizes */
  neuralNet(const Config& cfg, size_t inputSize, size_t outputSize);

  /* just as it says, randomizes ALL the weights of this neural network */
  void randomizeWeights();

  /* jitters the network's weight */
  void jitterWeights(float jitterMax);

  /* perform regression on an input set */
  template<class InputIterator>
  void feedForward(InputIterator current)
  {
    std::copy_n(current, inputBuffer.size(), inputBuffer.begin());
    feedForward();
  };

  void feedForward();

  void clearInput();
  
  /* deletes all elements within the neural network, invalidating it and freeing memory */
  void clear();

  bool isInitialized() const
  {
    return !model.is_empty();
  };

  FeatureVector::floatIterator_t inputBegin() { return inputBuffer.begin(); };
  FeatureVector::constFloatIterator_t inputBegin() const {
    return inputBuffer.begin();
  };

  FeatureVector::floatIterator_t inputEnd() { return inputBuffer.end(); };
  FeatureVector::constFloatIterator_t inputEnd() const {
    return inputBuffer.end();
  };

  FeatureVector::floatIterator_t outputBegin() { return outputBuffer.begin(); };
  FeatureVector::constFloatIterator_t outputBegin() const {
    return outputBuffer.begin();
  };

  FeatureVector::floatIterator_t outputEnd() { return outputBuffer.end(); };
  FeatureVector::constFloatIterator_t outputEnd() const {
    return outputBuffer.end();
  };

  size_t numInputs() const
  {
    return inputBuffer.size();
  };

  size_t numOutputs() const
  {
    return outputBuffer.size();
  };

  size_t getNumLayers() const
  {
    return layerWidths.size();
  };

  size_t getWidth(size_t iLayer) const
  {
    return layerWidths[iLayer];
  };

  torch::nn::Sequential& getModel() { return model; }
  const torch::nn::Sequential& getModel() const { return model; }

  /* output in binary to an ostream using torch::save */
  void output(std::ostream& oFile) const;

  /* input in binary from an istream using torch::load */
  bool input(std::istream& iFile);
  
}; // endOf class neuralNet

#endif /* NEURALNET_H */
