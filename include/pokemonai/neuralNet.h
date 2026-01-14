#ifndef NEURALNET_H
#define NEURALNET_H

#include "pokemonai/pkai.h"

#include "pokemonai/name.h"

#include <torch/torch.h>
#include <vector>
#include <memory>

class neuralNet;


class neuralNet: public Name
{
private:
  torch::nn::Sequential model;
  
  // Buffers for backward compatibility with iterators
  mutable std::vector<float> inputBuffer;
  mutable std::vector<float> outputBuffer;

  std::vector<size_t> layerWidths;

public:
  typedef std::vector<float>::iterator floatIterator_t;
  typedef std::vector<float>::const_iterator constFloatIterator_t;

  /* generates an empty invalid neural network */
  neuralNet() : model() {};

  /* creates an uninitialized neural network from an array of widths */
  template<class InputIterator>
  neuralNet(InputIterator current, const InputIterator last)
  {
    model = torch::nn::Sequential();
    size_t lastWidth = *current;
    layerWidths.push_back(lastWidth);
    inputBuffer.resize(lastWidth, 0.0f);
    
    auto it = current;
    ++it;
    for (; it != last; ++it)
    {
      size_t currentWidth = *it;
      layerWidths.push_back(currentWidth);
      model->push_back(torch::nn::Linear(lastWidth, currentWidth));
      model->push_back(torch::nn::Sigmoid()); // Maintaining sigmoid for compatibility
      lastWidth = currentWidth;
    }
    outputBuffer.resize(lastWidth, 0.0f);
  };

  /* return the result of outputneuron iOutputNode */
  float result(size_t iOutputNode) const
  {
    return *(outputBegin() + iOutputNode);
  };

  template<class OutputIterator>
  OutputIterator result_to(OutputIterator resultLoc) const
  {
    for (
      constFloatIterator_t cOutput = outputBegin(), 
      lOutput = outputEnd(); 
      cOutput != lOutput; )
    {
      *resultLoc++ = *cOutput++;
    }
    return resultLoc;
  };

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

  floatIterator_t inputBegin()			{ return inputBuffer.begin(); };
  constFloatIterator_t inputBegin() const	{ return inputBuffer.begin(); };

  floatIterator_t inputEnd()				{ return inputBuffer.end(); };
  constFloatIterator_t inputEnd() const	{ return inputBuffer.end(); };

  floatIterator_t outputBegin()			{ return outputBuffer.begin(); }; 
  constFloatIterator_t outputBegin() const{ return outputBuffer.begin(); }; 

  floatIterator_t outputEnd()				{ return outputBuffer.end(); };
  constFloatIterator_t outputEnd() const	{ return outputBuffer.end(); };

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

  /* output in binary to an ostream using torch::save */
  void output(std::ostream& oFile) const;

  /* input in binary from an istream using torch::load */
  bool input(std::istream& iFile);
  
}; // endOf class neuralNet

#endif /* NEURALNET_H */
