#ifndef TEMPORALPROP_NET_H
#define TEMPORALPROP_NET_H

#include "../inc/pkai.h"

#include <vector>
#include <assert.h>

#include "../inc/neuralNet.h"

#include "../inc/fp_compare.h"

class temporalpropNet;
class neuralNet;
struct neuron;

typedef std::vector< std::vector<neuron> >::iterator layerIterator_t;
typedef std::vector< std::vector<neuron> >::reverse_iterator rLayerIterator_t;
typedef std::vector<float>::iterator weightIterator_t;

class temporalpropSettings
{
public:
	static temporalpropSettings defaultSettings;

	/* learningRate constant */
	float learningRate;

	/* momentum constant */
	float momentum;

	/* maximum amount of jitter a weight can be given */
	float jitterMax;
	
	/* amount of feedback state t gets from future states */
	float lambda;

	/* temporal difference decaying factor */
	float gamma;

	/* lambda * gamma */
	float lambdaGamma;

	temporalpropSettings()
	{
		*this = defaultSettings;
	};

	temporalpropSettings(float _learningRate, float _momentum, float _jitterMax, float _lambda, float _gamma)
		: learningRate(_learningRate),
		momentum(_momentum),
		jitterMax(_jitterMax),
		lambda(_lambda),
		gamma(_gamma),
		lambdaGamma(_lambda * _gamma)
	{
	};
};

class temporalpropNet
{
private:
	/* network this bNet is working off of */
	neuralNet cNet;

	// size of set of all weights:
	std::vector<float> deltaWeights;
	std::vector<float> momentums;
	std::vector< std::vector<float> > eligibilities;
	// size of set of all neurons:
	std::vector<float> derivatives;
	// size of the output layer:
	std::vector<float> outputErrors;

	/* learning settings for this backpropNet */
	const temporalpropSettings* settings;

	void updateDeltaWeights();

	void updateEligibilities_layer(rLayerIterator_t nLayer, std::vector<float>& subEligibilities);
	void updateDerivatives_layer(rLayerIterator_t nLayer, std::vector<float>& subEligibilities);

public:
	temporalpropNet()
		: cNet(),
		deltaWeights(),
		momentums(),
		eligibilities(),
		derivatives(),
		outputErrors(),
		settings(&temporalpropSettings::defaultSettings)
	{
	};

	/* creates a backpropagation network with identical topology to targetNetwork */
	temporalpropNet(const neuralNet& targetNetwork, const temporalpropSettings& cSettings = temporalpropSettings::defaultSettings);

	/* given the desired output of a neuron that just produced a feed-forward result, propagates error */
	template<class InputIterator>
	float backPropagate( InputIterator cTestResult , bool updateElig = true)
	{
		//updateDerivatives();
		if (updateElig) { updateEligibilities(); }

		floatIterator_t cNeuronOutput = cNet.outputBegin();

		return temporalPropagate(cTestResult, cNeuronOutput, 0.0f);
	};

	/* given the desired output of a neuron that just produced a feed-forward result, propagates error */
	template<class InputIterator, class oInputIterator>
	float temporalPropagate( InputIterator cTestResult, oInputIterator cLastNeuronOutput, float _gamma = -1.0f )
	{
		// seed data into output layer, then call backPropagate_layer
		float meanSquaredError = 0.0f;
		float gamma = _gamma;
		if (_gamma < 0.0f) { gamma = settings->gamma; } 

		floatIterator_t cNeuronError = outputBegin();
		floatIterator_t cNeuronOutput = cNet.outputBegin();

		for (floatIterator_t lNeuronError = outputEnd(); 
			cNeuronError != lNeuronError; 
			++cNeuronError, ++cNeuronOutput, ++cTestResult, ++cLastNeuronOutput)
		{
			// error of the current output for state t in a series from 0..T
			// error[t] = reward[t+1] + gamma * output[t+1] - output[t]
			*cNeuronError = *cTestResult + (*cNeuronOutput * gamma) - *cLastNeuronOutput;

			// accumulate errorSquared:
			meanSquaredError += fastabs(*cNeuronError);

			// make sure the output value is no greater than 1 away from the test result (must be scaled beforehand)
			assert(mostlyLTE(fastabs(*cNeuronError), 2.0f));
		}
		
		updateDeltaWeights();
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
		return outputErrors.begin();
	};

	floatIterator_t outputEnd()
	{
		return outputErrors.end();
	};

	/* updates all eligibility running totals according ot lambda. If lambda is zero, equal to the current eligibility */
	void updateEligibilities();

	void zeroEligibilities();

	void zeroMomentums();

	/* updates all weights within the network after a backPropagate cycle */
	void updateWeights();

	/* randomizes the network */
	void randomizeWeights();

	/* jitters the network */
	void jitterNetwork();

	/* returns this backprop object's settings */
	const temporalpropSettings& getSettings() { return *settings; };

	/* returns a copy of this neural network */
	const neuralNet& getNeuralNet() const { return cNet; };
	neuralNet& getNeuralNet() { return cNet; };

}; // endOf class temporalpropNet

#endif /* TEMPORALPROP_NET_H */
