#ifndef BACKPROPNET_H
#define BACKPROPNET_H

#include "pokemonai/pkai.h"

#include <vector>
#include <assert.h>

#include "neuralNet.h"

#include "pokemonai/fp_compare.h"

class temporalpropSettings;
class backpropNet;
class neuralNet;
struct neuron;

typedef std::vector< std::vector<neuron> >::iterator layerIterator_t;
typedef std::vector< std::vector<neuron> >::reverse_iterator rLayerIterator_t;
typedef std::vector<float>::iterator weightIterator_t;

class backpropSettings
{
public:
  static backpropSettings defaultSettings;

  /* learningRate constant */
  float learningRate;

  /* momentum constant */
  float momentum;

  /* maximum amount of jitter a weight can be given */
  float jitterMax;

  backpropSettings(float _learningRate, float _momentum, float _jitterMax)
    : learningRate(_learningRate),
    momentum(_momentum),
    jitterMax(_jitterMax)
  {
  };

  backpropSettings(const temporalpropSettings& other);
};

class backpropNet
{
private:
  /* network this bNet is working off of */
  neuralNet cNet;

  // size of set of all weights:
  std::vector<float> deltaWeights;
  std::vector<float> momentums;
  std::vector<float> cErrors;

  /* learning settings for this backpropNet */
  const backpropSettings* settings;

  void backPropagate_layer(rLayerIterator_t nLayer);
  void updateDeltaWeights_layer(rLayerIterator_t nLayer);

public:
  backpropNet()
    : cNet(),
    deltaWeights(),
    momentums(),
    cErrors(),
    settings(&backpropSettings::defaultSettings)
  {
  };

  /* creates a backpropagation network with identical topology to targetNetwork */
  backpropNet(const neuralNet& targetNetwork, const backpropSettings& cSettings = backpropSettings::defaultSettings);

  /* given the desired output of a neuron that just produced a feed-forward result, propagates error */
  template<class InputIterator>
  float backPropagate( InputIterator cTestResult )
  {
    // seed data into output layer, then call backPropagate_layer
    float meanSquaredError = 0.0f; float currentError, absError;

    floatIterator_t cNeuronError = outputBegin();
    floatIterator_t cNeuronOutput = cNet.outputBegin();

    for (floatIterator_t lNeuronError = outputEnd(); 
      cNeuronError != lNeuronError; 
      ++cNeuronError, ++cNeuronOutput, ++cTestResult)
    {
      // error of the current output
      currentError = *cTestResult - *cNeuronOutput;

      // weight error by this neuron's activation (fully activated or deactivated means no change)
      neuralNet::activationPrime(&*cNeuronOutput, &*cNeuronError);
      *cNeuronError *= currentError;

      // accumulate absolute error:
      absError = fastabs(currentError);
      meanSquaredError += absError;
      // make sure the output value is no greater than 1 away from the test result (must be scaled beforehand)
      assert(mostlyLTE(absError, 1.0f));
    }
    
    backPropagate();
    return meanSquaredError;
  };

  size_t iOLayerBegin()
  {
    return cNet.net.back().front().iNeuronIndex;
  };

  size_t iOLayerEnd()
  {
    return cNet.net.back().back().iNeuronIndex + 1;
  };

  floatIterator_t outputBegin()
  {
    return cErrors.begin() + iOLayerBegin();
  };

  floatIterator_t outputEnd()
  {
    return cErrors.begin() + iOLayerEnd();
  };

  /* propagates error through the network */
  void backPropagate();

  void zeroMomentums();

  /* updates all weights within the network after a backPropagate cycle */
  void updateWeights();

  /* randomizes the network */
  void randomizeWeights();

  /* jitters the network */
  void jitterNetwork();

  /* returns this backprop object's settings */
  const backpropSettings& getSettings() { return *settings; };

  /* returns a copy of this neural network */
  const neuralNet& getNeuralNet() const { return cNet; };
  neuralNet& getNeuralNet() { return cNet; };

  friend struct backpropNeuron;
}; // endOf class backpropNet

#endif /* BACKPROPNET_H */
