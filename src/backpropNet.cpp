
#include "../inc/backpropNet.h"

#ifdef WIN32
#include <xmmintrin.h>
#else // probably linux
#include <x86intrin.h>
#endif

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
	__m128 vMomentumConstant;
	__m128 vLearnRate;
	__m128 vSub0, vSub1;

	vLearnRate = _mm_load_ps1(&settings->learningRate);
	vMomentumConstant = _mm_load_ps1(&settings->momentum);

	// update weights, reset weight accumulator:
	for (__m128* vWeight = (__m128*)&cNet.weights.front(),
		*vDeltaWeight = (__m128*)&deltaWeights.front(),
		*vMomentum = (__m128*)&momentums.front(),
		*vEndWeight = (__m128*)&cNet.weights.back();
		vWeight != vEndWeight; 
		++vWeight, ++vDeltaWeight, ++vMomentum)
	{
		// scale current momentum by momentum constant
		vSub0 = _mm_mul_ps(*vMomentum, vMomentumConstant);
		// multiply aggregate error by learning rate
		vSub1 = _mm_mul_ps(*vDeltaWeight, vLearnRate);
		// add momentum to learning rate
		vSub0 = _mm_add_ps(vSub0, vSub1);
		// save momentum:
		_mm_store_ps((float*)vMomentum, vSub0);
		// update weight:
		*vWeight = _mm_add_ps(*vWeight, vSub0);

		// zero delta weight:
		*vDeltaWeight = _mm_setzero_ps();
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
	float* lVectorError = lError - ((lError - cError)&3); // equivalent to mod 4
	__m128 vActPrime;

	// aligned deactivations:
	while (cError != lVectorError)
	{
		// perform activation prime, store to vActPrime:
		neuralNet::activationPrime_sse(cActivation, (float*)&vActPrime);
		// multiply error of neuron by actPrime:
		vActPrime = _mm_mul_ps((*(__m128*)cError), vActPrime);
		// store modified error back to pointer:
		_mm_store_ps(cError, vActPrime);
		// increment pointers:
		cError += 4;
		cActivation += 4;
	};

	// postscript: (unaligned activations):
	{
		size_t dError = (lError - cError);
		float activationPrime;
		switch(dError)
		{
		case 3:
			neuralNet::activationPrime(cActivation + 2, &activationPrime);
			cError[2] *= activationPrime;
		case 2:
			neuralNet::activationPrime(cActivation + 1, &activationPrime);
			cError[1] *= activationPrime;
		case 1:
			neuralNet::activationPrime(cActivation + 0, &activationPrime);
			cError[0] *= activationPrime;
		case 0:
			break;
		}
	}
};

void backpropNet::updateDeltaWeights_layer(rLayerIterator_t nLayer)
{
	floatIterator_t cWeight, lWeight, cDeltaWeight, pActivation;
	neuronIterator_t cNeuron = nLayer->begin();
	__m128 *vDeltaWeight, *vPActivation, *vEndDeltaWeight;
	__m128 vNeuronError, vSub0, vSub1, vSub2, vSub3;

	for (neuronIterator_t lNeuron = nLayer->end(); 
		cNeuron != lNeuron; 
		++cNeuron)
	{
		// modified error of k neuron
		float& cNeuronError = cErrors[cNeuron->iNeuronIndex];
		// activation of j neuron
		vPActivation = (__m128*)&*(cNet.activations.begin() + (nLayer + 1)->front().iNeuronIndex);
		// deltaWeights dW[j,k]
		vDeltaWeight = (__m128*)&*(deltaWeights.begin() + cNeuron->iWeightBegin);
		vEndDeltaWeight = (__m128*)&*(deltaWeights.begin() + cNeuron->iWeightEnd);

		vNeuronError = _mm_load_ps1(&cNeuronError);

		// update each neuron's weight:
		do
		{
			/* each neuron's update modification is:
			 * dWeight[j,k] += activation[j] * modifiedError[k]
			 */
			// multiply:
			vSub0 = _mm_mul_ps(vPActivation[0], vNeuronError);
			vSub1 = _mm_mul_ps(vPActivation[1], vNeuronError);
			vSub2 = _mm_mul_ps(vPActivation[2], vNeuronError);
			vSub3 = _mm_mul_ps(vPActivation[3], vNeuronError);

			// add to error accumulators:
			vDeltaWeight[0] = _mm_add_ps(vDeltaWeight[0], vSub0);
			vDeltaWeight[1] = _mm_add_ps(vDeltaWeight[1], vSub1);
			vDeltaWeight[2] = _mm_add_ps(vDeltaWeight[2], vSub2);
			vDeltaWeight[3] = _mm_add_ps(vDeltaWeight[3], vSub3);

			// increment pointers:
			vDeltaWeight += 4;
			vPActivation += 4;
		}while(vDeltaWeight != vEndDeltaWeight);

		// update bias weight: (activation of this neuron is always 1, so we can skip multiply by vPActivation)
		vSub0 = _mm_add_ss(vDeltaWeight[0], vNeuronError);
		_mm_store_ss((float*)vDeltaWeight, vSub0);
	}
};





void backpropNet::jitterNetwork()
{
	cNet.jitterWeights(settings->jitterMax);
};
