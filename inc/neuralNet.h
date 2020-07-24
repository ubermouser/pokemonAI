#ifndef NEURALNET_H
#define NEURALNET_H

#include "../inc/pkai.h"

#include "../inc/name.h"

#include <assert.h>
#include <vector>

struct neuron;

typedef std::vector< std::vector<neuron> >::iterator layerIterator_t;
typedef std::vector< std::vector<neuron> >::const_iterator constLayerIterator_t;
typedef std::vector<neuron>::iterator neuronIterator_t;
typedef std::vector<neuron>::const_iterator constNeuronIterator_t;
typedef std::vector<float>::const_iterator constFloatIterator_t;
typedef std::vector<float>::iterator floatIterator_t;

class neuralNet;

struct neuron
{
public:
	/* weights of this neuron from all neurons of the previous row. Weight at the end is the bias weight */
	size_t iWeightBegin;
	size_t iWeightEnd;

	/* index of the activation / input of this neuron */
	size_t iNeuronIndex;

	size_t numWeights() const { return (iWeightEnd - iWeightBegin)>0?(iWeightEnd - iWeightBegin)+1:0; }

	float& getWeight(neuralNet& net, size_t iWeight) const;
	const float& getWeight(const neuralNet& net, size_t iWeight) const;

	floatIterator_t weightsBegin(neuralNet& parent) const;
	floatIterator_t weightsEnd(neuralNet& parent) const;

	constFloatIterator_t weightsBegin(const neuralNet& parent) const;
	constFloatIterator_t weightsEnd(const neuralNet& parent) const;

	neuron(neuralNet& parent, size_t numWeights);
};

class neuralNet: public name
{
private:
	/* highest level is each row, each row has a column, and 
	 * each neuron has a vector of the size of the column before it + 1 
	 */
	std::vector< std::vector<neuron> > net;

	std::vector<float> weights;
	std::vector<float> activations;

	void feedForward_layer(layerIterator_t cLayer);

	/* activation function */
	static float activation(float neuronOutput);

	static float activation_approx(float neuronOutput);
	static void activation_approx(const float* neuronOutput, float* result);

	/* inverse of the activation function */
	static float activationPrime(float neuronOutput);
	static void activationPrime(const float* neuronOutput, float* result);
public:
	/* generates an empty invalid neural network */
	neuralNet() 
		: net(),
		weights(),
		activations()
	{ 
	};

	/* creates an uninitialized neural network from an array of widths */
	template<class InputIterator>
	neuralNet( InputIterator current, const InputIterator last )
		: net()
	{
		assert((last - current) <= MAXNETWORKLAYERS && (last - current) >= 2U);
		// reserve size to number of elements between last and first:
		net.clear();
		net.reserve(last - current);

		// todo: make sure weights, activation, input begin on 16 byte word boundary

		// foreach layer:
		size_t lastLayerNeurons = 0;
		for (; current != last; ++current)
		{
			assert(*current > 0 && *current <= MAXNETWORKWIDTH);
			assert(lastLayerNeurons % 16 == 0); // input and hidden layer neurons must be a multiple of 16

			// push back new layer of neurons
			net.push_back(std::vector<neuron>());

			// initialize to proper number of neurons, 
			//number of inputs equivalent to last layer number of neurons (fully connected)
			net.back().reserve(*current);
			activations.reserve(activations.size() + *current);
			for(size_t iNeuron = 0; iNeuron != *current; ++iNeuron)
			{
				net.back().push_back(neuron(*this, lastLayerNeurons));
			}

			lastLayerNeurons = *current;
		}
	};

	/* return the result of outputneuron iOutputNode */
	float result(size_t iOutputNode)
	{
		return *(outputBegin() + iOutputNode);
	};

	template<class OutputIterator>
	OutputIterator result(OutputIterator resultLoc)
	{
		for (
			floatIterator_t cOutput = outputBegin(), 
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
	void feedForward( InputIterator current)
	{
		// seed data into network, then call feedForward_layer
		for (
			floatIterator_t cInput = inputBegin(), 
			lInput = inputEnd(); 
			cInput != lInput; )
		{
			*cInput++ = *current++;
		}

		feedForward();
	};

	void feedForward();

	void clearInput();
	
	/* deletes all elements within the neural network, invalidating it and freeing memory */
	void clear();

	bool isInitialized() const
	{
		return (!net.empty() && !weights.empty() && !activations.empty());
	};

	floatIterator_t inputBegin()			{ return activations.begin() + net.front().front().iNeuronIndex; };
	constFloatIterator_t inputBegin() const	{ return activations.begin() + net.front().front().iNeuronIndex; };

	floatIterator_t inputEnd()				{ return activations.begin() + (net.front().back().iNeuronIndex + 1); };
	constFloatIterator_t inputEnd() const	{ return activations.begin() + (net.front().back().iNeuronIndex + 1); };

	floatIterator_t outputBegin()			{ return activations.begin() + net.back().front().iNeuronIndex; }; 
	constFloatIterator_t outputBegin() const{ return activations.begin() + net.back().front().iNeuronIndex; }; 

	floatIterator_t outputEnd()				{ return activations.begin() + (net.back().back().iNeuronIndex + 1); };
	constFloatIterator_t outputEnd() const	{ return activations.begin() + (net.back().back().iNeuronIndex + 1); };

	size_t numInputs() const
	{
		return net.front().size();
	};

	size_t numOutputs() const
	{
		if (net.empty()) { return 0; } 
		return net.back().size();
	};

	size_t getNumLayers() const
	{
		return net.size();
	};

	size_t getWidth(size_t iLayer) const
	{
		return net[iLayer].size();
	};

	/* output in plaintext to an ostream */
	void output(std::ostream& oFile, bool printHeader = true) const;

	/* input in plaintext from a string */
	bool input(const std::vector<std::string>& lines, size_t& firstLine);
	
	friend class backpropNet;
	friend class temporalpropNet;
	friend struct neuron;
}; // endOf class neuralNet

#endif /* NEURALNET_H */
