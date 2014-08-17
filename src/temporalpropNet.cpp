
#include "../inc/temporalpropNet.h"

#include <math.h>
#include <boost/math/special_functions/fpclassify.hpp>

#include <boost/foreach.hpp>

#include "../inc/neuralNet.h"
#include "../inc/fp_compare.h"

temporalpropSettings temporalpropSettings::defaultSettings(0.1f, 0.01f, 0.25f, 0.7f, 0.6f);





temporalpropNet::temporalpropNet(const neuralNet& targetNetwork, const temporalpropSettings& cSettings)
	: cNet(targetNetwork),
	deltaWeights(targetNetwork.weights.size(), 0.0f),
	momentums(targetNetwork.weights.size(), 0.0f),
	eligibilities(targetNetwork.numOutputs()),
	derivatives(targetNetwork.activations.size(), 0.0f),
	outputErrors(targetNetwork.numOutputs(), 0.0f),
	settings(&cSettings)
{
	BOOST_FOREACH(std::vector<float>& cEligibility, eligibilities)
	{
		cEligibility.assign(targetNetwork.weights.size(), 0.0f);
	}
};





void temporalpropNet::zeroMomentums()
{
	memset(&momentums.front(), 0, sizeof(float)*momentums.size());
};





void temporalpropNet::randomizeWeights()
{
	cNet.randomizeWeights();
};





void temporalpropNet::updateWeights()
{
	// update weights, reset weight accumulator:
	for (floatIterator_t cWeight = cNet.weights.begin(),
		cDeltaWeight = deltaWeights.begin(),
		cMomentum = momentums.begin(),
		endWeight = cNet.weights.end(); 
		cWeight != endWeight; 
		++cWeight, ++cDeltaWeight, ++cMomentum)
	{
		*cMomentum = settings->learningRate * *cDeltaWeight + (settings->momentum * *cMomentum);
		*cWeight += *cMomentum;
		*cDeltaWeight = 0.0f;
		assert(!boost::math::isnan(*cWeight));
	}
};





void temporalpropNet::updateDeltaWeights()
{
	// output layer has a special case, as it's directly connected to the output neurons:
	for (size_t iOutput = 0, iSize = cNet.numOutputs(); iOutput != iSize; ++iOutput)
	{
		/*for (floatIterator_t cDeltaWeight = deltaWeights.begin() + cNet.net.back()[iOutput].iWeightBegin,
			endDeltaWeight = deltaWeights.begin() + (cNet.net.back()[iOutput].iWeightEnd + 1),
			cEligibility = eligibilities[iOutput].begin() + cNet.net.back()[iOutput].iWeightBegin;
			cDeltaWeight != endDeltaWeight;
			++cDeltaWeight, ++cEligibility)
		{
			*cDeltaWeight += *cEligibility * outputErrors[iOutput];
		}*/

		// update deltaWeights based on eligibility and output:
		for (floatIterator_t cDeltaWeight = deltaWeights.begin(),
			cEligibility = eligibilities[iOutput].begin(),
			endDeltaWeight = deltaWeights.end() /*+ cNet.net.back().front().iWeightBegin*/;
			cDeltaWeight != endDeltaWeight; 
			++cDeltaWeight, ++cEligibility)
		{
			*cDeltaWeight += *cEligibility * outputErrors[iOutput];
		}
	}
};





void temporalpropNet::zeroEligibilities()
{
	/*BOOST_FOREACH(float& cEligibility, eligibilities)
	{
		cEligibility = 0.0f;
	}*/
	BOOST_FOREACH(std::vector<float>& cEligibility, eligibilities)
	{
		memset(&cEligibility.front(), 0, sizeof(float)*cEligibility.size());
	}
	
}

void temporalpropNet::updateEligibilities()
{
	for (size_t iOutput = 0; iOutput != outputErrors.size(); ++iOutput)
	{
		// seed output layer eligibility with derivatives:
		size_t iNOutput = 0;
		for (floatIterator_t cDerivative = derivatives.begin() + iOLayerBegin(), 
			cActivation = cNet.activations.begin() + iOLayerBegin(),
			lDerivative = derivatives.begin() + iOLayerEnd();
			cDerivative != lDerivative;
			++cDerivative, ++cActivation, ++iNOutput)
		{
			if (iNOutput != iOutput) { *cDerivative = 0.0f; }
			else { *cDerivative = neuralNet::activationPrime(*cActivation); }
		}

		rLayerIterator_t cLayer = cNet.net.rbegin()+1;
		// run each layer in series:
		for (rLayerIterator_t lLayer = cNet.net.rend(); cLayer != lLayer; ++cLayer)
		{
			updateDerivatives_layer(cLayer, eligibilities[iOutput]);
		}
		cLayer = cNet.net.rbegin();
		// update the eligibilities of each layer in series:
		for (rLayerIterator_t lLayer = cNet.net.rend()-1; cLayer != lLayer; ++cLayer)
		{
			updateEligibilities_layer(cLayer, eligibilities[iOutput]);
		}
	}
}

void temporalpropNet::updateEligibilities_layer(rLayerIterator_t nLayer, std::vector<float>& subEligibilities)
{
	neuronIterator_t cNeuron = nLayer->begin();
	// finalize eligibility of previous layer:
	for (neuronIterator_t lNeuron = nLayer->end(); 
		cNeuron != lNeuron; 
		++cNeuron)
	{

		// foreach [previous neuron, eligbility [p,c] ], up to endEligibility
		neuronIterator_t pNeuron = (nLayer + 1)->begin();
		neuronIterator_t pEnd = (nLayer + 1)->end();
		floatIterator_t cEligibility = subEligibilities.begin() + cNeuron->iWeightBegin;

		float& cDerivative = derivatives[cNeuron->iNeuronIndex];

		// summate all neurons of next layer:
		do // TODO: SSE instructions by doing 4 multiplies at once? -- this method is not performance critical right now
		{
			const float& pActivation = cNet.activations[pNeuron->iNeuronIndex];
			/*
			 * next layer Eligibility finalizer is:
			 * e[j,i] = e[j,i]*(gamma*lambda) + runningDerivative[i] * activation[j]
			 */
			*cEligibility =
				*cEligibility * settings->lambdaGamma
				+ cDerivative * pActivation;

			++pNeuron;
			++cEligibility;
		}
		while (pNeuron != pEnd);

		// bias eligibility:
		*cEligibility = *cEligibility * settings->lambdaGamma + cDerivative;
	}
}

void temporalpropNet::updateDerivatives_layer(rLayerIterator_t nLayer, std::vector<float>& subEligibilities)
{
	// propagate derivatives one level down:
	neuronIterator_t cNeuron = nLayer->begin();
	size_t iPreviousNeuron = 0;
	for (neuronIterator_t lNeuron = nLayer->end(); 
		cNeuron != lNeuron; 
		++cNeuron, ++iPreviousNeuron)
	{

		// foreach [current neuron, next weight, next running derivative], up to endWeight
		neuronIterator_t nNeuron = (nLayer - 1)->begin();
		neuronIterator_t nEnd = (nLayer - 1)->end();

		float& cDerivative = derivatives[cNeuron->iNeuronIndex];
		const float& cActivation = cNet.activations[cNeuron->iNeuronIndex];

		// zero this neuron's running derivative accumulator:
		cDerivative = 0.0f;

		// summate all neurons of next layer:
		do // TODO: SSE instructions by doing 4 multiplies at once? -- this method is not performance critical right now
		{
			float& nDerivative = derivatives[nNeuron->iNeuronIndex];
			
			/*
			 * current layer Eligibility update is:
			 * runningDerivative[j] = runningDerivative[i] * weight[j,i]
			 */
			cDerivative += nDerivative * nNeuron->weightsBegin(cNet)[iPreviousNeuron];

			++nNeuron;
		}
		while (nNeuron != nEnd);

		// finalize current running derivative:
		cDerivative *= neuralNet::activationPrime(cActivation);
	}
}





void temporalpropNet::jitterNetwork()
{
	cNet.jitterWeights(settings->jitterMax);
};
