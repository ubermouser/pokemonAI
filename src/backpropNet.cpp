
#include "../inc/backpropNet.h"

#include <math.h>
#include <boost/math/special_functions/fpclassify.hpp>

#include <boost/foreach.hpp>

#include "../inc/temporalpropNet.h"
#include "../inc/neuralNet.h"
#include "../inc/fp_compare.h"

backpropSettings backpropSettings::defaultSettings(0.3f, 0.2f, 0.25f);

backpropSettings::backpropSettings(const temporalpropSettings& tSettings)
  : learningRate(tSettings.learningRate),
  momentum(tSettings.momentum),
  jitterMax(tSettings.jitterMax)
{
};





backpropNet::backpropNet(const neuralNet& targetNetwork, const backpropSettings& cSettings)
  : cNet(targetNetwork),
  deltaWeights(targetNetwork.weights.size(), 0.0f),
  momentums(targetNetwork.weights.size(), 0.0f),
  cErrors(targetNetwork.activations.size(), 0.0f),
  settings(&cSettings)
{
};





void backpropNet::zeroMomentums()
{
  memset(&momentums.front(), 0, sizeof(float)*momentums.size());
};





void backpropNet::randomizeWeights()
{
  cNet.randomizeWeights();
};





void backpropNet::updateWeights()
{
  float vMomentumConstant;
  float vLearnRate;
  float vSub0, vSub1;

  vLearnRate = settings->learningRate;
  vMomentumConstant = settings->momentum;

  // update weights, reset weight accumulator:
  for (float* vWeight = (float*)&cNet.weights.front(),
    *vDeltaWeight = (float*)&deltaWeights.front(),
    *vMomentum = (float*)&momentums.front(),
    *vEndWeight = (float*)&cNet.weights.back();
    vWeight != vEndWeight; 
    ++vWeight, ++vDeltaWeight, ++vMomentum)
  {
    // scale current momentum by momentum constant
    vSub0 = *vMomentum * vMomentumConstant;
    // multiply aggregate error by learning rate
    vSub1 = *vDeltaWeight * vLearnRate;
    // add momentum to learning rate
    vSub0 = vSub0 + vSub1;
    // save momentum:
    *vMomentum = vSub0;
    // update weight:
    *vWeight = *vWeight + vSub0;

    // zero delta weight:
    *vDeltaWeight = 0;
  }
};




void backpropNet::backPropagate()
{
  rLayerIterator_t cLayer = cNet.net.rbegin()+1;
  // run each layer of backpropagation in series:
  for (rLayerIterator_t lLayer = cNet.net.rend(); cLayer != lLayer; ++cLayer)
  {
    backPropagate_layer(cLayer);
  }
  cLayer = cNet.net.rbegin();
  // update the delta weights of each layer in series:
  for (rLayerIterator_t lLayer = cNet.net.rend()-1; cLayer != lLayer; ++cLayer)
  {
    updateDeltaWeights_layer(cLayer);
  }
};

void backpropNet::backPropagate_layer(rLayerIterator_t nLayer)
{
  neuronIterator_t cNeuron = nLayer->begin();
  
  // propagate error one level down:
  size_t iPreviousNeuron = 0;
  for (neuronIterator_t lNeuron = nLayer->end(); 
    cNeuron != lNeuron; 
    ++cNeuron, ++iPreviousNeuron)
  {

    // foreach [current neuron, next weight, next error], up to endWeight
    neuronIterator_t nNeuron = (nLayer - 1)->begin();
    neuronIterator_t nEnd = (nLayer - 1)->end();

    // set this neuron's accumulator
    float& cNeuronError = cErrors[cNeuron->iNeuronIndex];

    // zero this neuron's accumulator:
    cNeuronError = 0.0f;

    // summate all neurons of next layer:
    do // TODO: SSE instructions by doing 4 multiplies at once? -- this method is the most performance critical portion of backPropagate
    {
      /* each neuron's error contribution is:
       * error[i] += weight[i,j] * error[j]
       */
      cNeuronError += nNeuron->weightsBegin(cNet)[iPreviousNeuron] * cErrors[nNeuron->iNeuronIndex];

      ++nNeuron;
    }
    while (nNeuron != nEnd);
  }

  // calculate modified error of all neurons in this layer:
  const float* cActivation = (&cNet.activations.front() + nLayer->front().iNeuronIndex);
  float* cError = (&cErrors.front() + nLayer->front().iNeuronIndex);
  float* lError = (&cErrors.front() + nLayer->back().iNeuronIndex + 1);
  float vActPrime;

  // deactivations:
  float activationPrime;
  while (cError != lError)
  {
    // perform activation prime, store to vActPrime:
    neuralNet::activationPrime(cActivation, &activationPrime);
    // store modified error back to pointer:
    *cError *= activationPrime;
    // increment pointers:
    cError += 1;
    cActivation += 1;
  };
};

void backpropNet::updateDeltaWeights_layer(rLayerIterator_t nLayer)
{
  floatIterator_t cWeight, lWeight, cDeltaWeight, pActivation;
  neuronIterator_t cNeuron = nLayer->begin();
  float *vDeltaWeight, *vPActivation, *vEndDeltaWeight;
  float vNeuronError, vSub0, vSub1, vSub2, vSub3;

  for (neuronIterator_t lNeuron = nLayer->end(); 
    cNeuron != lNeuron; 
    ++cNeuron)
  {
    // modified error of k neuron
    float& cNeuronError = cErrors[cNeuron->iNeuronIndex];
    // activation of j neuron
    vPActivation = (float*)&*(cNet.activations.begin() + (nLayer + 1)->front().iNeuronIndex);
    // deltaWeights dW[j,k]
    vDeltaWeight = (float*)&*(deltaWeights.begin() + cNeuron->iWeightBegin);
    vEndDeltaWeight = (float*)&*(deltaWeights.begin() + cNeuron->iWeightEnd);

    vNeuronError = cNeuronError;

    // update each neuron's weight:
    do
    {
      /* each neuron's update modification is:
       * dWeight[j,k] += activation[j] * modifiedError[k]
       */
      // multiply:
      vSub0 = vPActivation[0] * vNeuronError;
      vSub1 = vPActivation[1] * vNeuronError;
      vSub2 = vPActivation[2] * vNeuronError;
      vSub3 = vPActivation[3] * vNeuronError;

      // add to error accumulators:
      vDeltaWeight[0] = vDeltaWeight[0] + vSub0;
      vDeltaWeight[1] = vDeltaWeight[1] + vSub1;
      vDeltaWeight[2] = vDeltaWeight[2] + vSub2;
      vDeltaWeight[3] = vDeltaWeight[3] + vSub3;

      // increment pointers:
      vDeltaWeight += 4;
      vPActivation += 4;
    }while(vDeltaWeight != vEndDeltaWeight);

    // update bias weight: (activation of this neuron is always 1, so we can skip multiply by vPActivation)
    vSub0 = vDeltaWeight[0] + vNeuronError;
    *vDeltaWeight = vSub0;
  }
};





void backpropNet::jitterNetwork()
{
  cNet.jitterWeights(settings->jitterMax);
};
