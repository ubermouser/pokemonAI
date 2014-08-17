//#define PKAI_IMPORT
#include "../inc/neuralNet.h"

#include <math.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <cstring>

#ifdef WIN32
#include <xmmintrin.h>
#else // probably linux
#include <x86intrin.h>
#endif

// we need a define rule to determine whether the supporting system has AVX
#include <avxintrin-emu.h>

#include <boost/foreach.hpp>
#include <boost/static_assert.hpp>

#include <iomanip>
#include "../inc/fp_compare.h"
#include "../inc/init_toolbox.h"

static const std::string header = "PKNNT2";

#define ALIGN_SIZE 8
#define ALIGN_OF(size) (size & (ALIGN_SIZE-1))
#define IS_ALIGNED(size) (ALIGN_OF(size) == 0)

neuron::neuron(neuralNet& parent, size_t numWeights)
	: iWeightBegin(SIZE_MAX),
	iWeightEnd(SIZE_MAX),
	iNeuronIndex(SIZE_MAX)
{
	size_t _iWeightBegin = parent.weights.size();
	if (!IS_ALIGNED(_iWeightBegin)) { _iWeightBegin += ALIGN_SIZE - ALIGN_OF(_iWeightBegin); } 
	// set weight array begin and end:
	iWeightBegin = _iWeightBegin;
	iWeightEnd = iWeightBegin + numWeights; // allow 1 value for bias weight at the end

	parent.weights.resize(iWeightBegin + ((numWeights>0)?(numWeights+1):0), 0.0f);
	// always one value allocated for activations:
	iNeuronIndex = parent.activations.size();
	parent.activations.push_back(0.0f);
};

float& neuron::getWeight(neuralNet& net, size_t iWeight) const { return *(weightsBegin(net) + iWeight); };
const float& neuron::getWeight(const neuralNet& net, size_t iWeight) const { return *(weightsBegin(net) + iWeight); };

floatIterator_t neuron::weightsBegin(neuralNet& parent) const { return parent.weights.begin() + iWeightBegin; };
floatIterator_t neuron::weightsEnd(neuralNet& parent) const { return parent.weights.begin() + iWeightEnd; };

constFloatIterator_t neuron::weightsBegin(const neuralNet& parent) const { return parent.weights.begin() + iWeightBegin; };
constFloatIterator_t neuron::weightsEnd(const neuralNet& parent) const { return parent.weights.begin() + iWeightEnd; };





float neuralNet::activation(float neuronOutput)
{
#define SIGMOIDCOEFFICIENT 4.0f

	// TODO: if this needs to be faster, gnubg uses a jump table for estimating the exp function
	if (neuronOutput < -10.0f)
	{
		return 1.0f / 19931.370438230298f;
	}
	else if (neuronOutput > 10.0f)
	{
		return 19930.370438230298f / 19931.370438230298f;
	}
	else
	{
		return (1.0f / (1.0f + exp(-1.0f*SIGMOIDCOEFFICIENT*neuronOutput)));
	}
};

float neuralNet::activation_approx(float neuronOutput)
{
	activation_approx(&neuronOutput, &neuronOutput);
	return neuronOutput;
};

void neuralNet::activation_approx(const float* _neuronOutput, float* result)
{
	BOOST_STATIC_ASSERT(SIGMOIDCOEFFICIENT == 4.0f);
	// code from http://ybeernet.blogspot.com/2011/03/speeding-up-sigmoid-function-by.html
	// approximates sigmoid function with coefficient 4.0f
	float tmp = std::min(*_neuronOutput, 4.0f);
	tmp = 1.0f - 0.25f * tmp;
	tmp *= tmp;
	tmp *= tmp;
	tmp *= tmp;
	tmp *= tmp;
	tmp = 1.0f / (1.0f + tmp);

	assert(fastabs(tmp - activation(*_neuronOutput)) < 0.05f);

	// return ans
	*result = tmp;
};

void neuralNet::activation_approx_sse(const float* _neuronOutput, float* result)
{
	BOOST_STATIC_ASSERT(SIGMOIDCOEFFICIENT == 4.0f);
	// code adapted from http://ybeernet.blogspot.com/2011/03/speeding-up-sigmoid-function-by.html
	// approximates sigmoid function with coefficient 4.0f
	static const __m128 ones = _mm_set1_ps(1.0f);
	static const __m128 oneFourths = _mm_set1_ps(0.25f);
	static const __m128 fours = _mm_set1_ps(4.0f);

	__m128 temp;
	const __m128* vOutput = (__m128*)_neuronOutput;

	// min (output, 4.0)
	temp = _mm_min_ps(*vOutput, fours);
	// multiply by 0.25
	temp = _mm_mul_ps(temp, oneFourths);
	// 1 - ans
	temp = _mm_sub_ps(ones, temp);
	// ans^16
	temp = _mm_mul_ps(temp, temp);
	temp = _mm_mul_ps(temp, temp);
	temp = _mm_mul_ps(temp, temp);
	temp = _mm_mul_ps(temp, temp);
	// 1 + ans
	temp = _mm_add_ps(ones, temp);
	// 1 / ans
	temp = _mm_rcp_ps(temp);

#ifndef NDEBUG
	const float* _temp = (float*)&temp;
	assert(fastabs(_temp[0] - activation(_neuronOutput[0])) < 0.05f);
	assert(fastabs(_temp[1] - activation(_neuronOutput[1])) < 0.05f);
	assert(fastabs(_temp[2] - activation(_neuronOutput[2])) < 0.05f);
	assert(fastabs(_temp[3] - activation(_neuronOutput[3])) < 0.05f);
#endif

	// return ans
	_mm_store_ps(result, temp);
};

void neuralNet::activation_approx_avx(const float* _neuronOutput, float* result)
{
	BOOST_STATIC_ASSERT(SIGMOIDCOEFFICIENT == 4.0f);
	// code adapted from http://ybeernet.blogspot.com/2011/03/speeding-up-sigmoid-function-by.html
	// approximates sigmoid function with coefficient 4.0f
	static const __m256 ones = _mm256_set1_ps(1.0f);
	static const __m256 oneFourths = _mm256_set1_ps(0.25f);
	static const __m256 fours = _mm256_set1_ps(4.0f);

	__m256 temp;
	const __m256* vOutput = (__m256*)_neuronOutput;

	// min (output, 4.0)
	temp = _mm256_min_ps(*vOutput, fours);
	// multiply by 0.25
	temp = _mm256_mul_ps(temp, oneFourths);
	// 1 - ans
	temp = _mm256_sub_ps(ones, temp);
	// ans^16
	temp = _mm256_mul_ps(temp, temp);
	temp = _mm256_mul_ps(temp, temp);
	temp = _mm256_mul_ps(temp, temp);
	temp = _mm256_mul_ps(temp, temp);
	// 1 + ans
	temp = _mm256_add_ps(ones, temp);
	// 1 / ans
	temp = _mm256_rcp_ps(temp);

#ifndef NDEBUG
	const float* _temp = (float*)&temp;
	assert(fastabs(_temp[0] - activation(_neuronOutput[0])) < 0.05f);
	assert(fastabs(_temp[1] - activation(_neuronOutput[1])) < 0.05f);
	assert(fastabs(_temp[2] - activation(_neuronOutput[2])) < 0.05f);
	assert(fastabs(_temp[3] - activation(_neuronOutput[3])) < 0.05f);
	assert(fastabs(_temp[4] - activation(_neuronOutput[4])) < 0.05f);
	assert(fastabs(_temp[5] - activation(_neuronOutput[5])) < 0.05f);
	assert(fastabs(_temp[6] - activation(_neuronOutput[6])) < 0.05f);
	assert(fastabs(_temp[7] - activation(_neuronOutput[7])) < 0.05f);
#endif

	// return ans
	_mm256_store_ps(result, temp);
};




float neuralNet::activationPrime(float neuronOutput)
{
	activationPrime(&neuronOutput, &neuronOutput);
	return neuronOutput;
};

void neuralNet::activationPrime(const float* neuronOutput, float* result)
{
	*result = SIGMOIDCOEFFICIENT * *neuronOutput * (1.0f - *neuronOutput);
};

void neuralNet::activationPrime_sse(const float* neuronOutput, float* result)
{
	static const __m128 ones = _mm_set1_ps(1.0f);
	static const __m128 sigCoefficients = _mm_set1_ps(SIGMOIDCOEFFICIENT);

	__m128 temp;
	const __m128* vOutput = (__m128*)neuronOutput;

	// 1 - ans
	temp = _mm_sub_ps(ones, *vOutput);
	// (1-ans) * ans
	temp = _mm_mul_ps(temp, *vOutput);
	// ans * coefficient
	temp = _mm_mul_ps(temp, sigCoefficients);

#ifndef NDEBUG
	const float* _temp = (float*)&temp;
	assert(fastabs(_temp[0] - activationPrime(neuronOutput[0])) < 0.05f);
	assert(fastabs(_temp[1] - activationPrime(neuronOutput[1])) < 0.05f);
	assert(fastabs(_temp[2] - activationPrime(neuronOutput[2])) < 0.05f);
	assert(fastabs(_temp[3] - activationPrime(neuronOutput[3])) < 0.05f);
#endif

	// return ans
	_mm_store_ps(result, temp);
};

void neuralNet::activationPrime_avx(const float* neuronOutput, float* result)
{
	static const __m256 ones = _mm256_set1_ps(1.0f);
	static const __m256 sigCoefficients = _mm256_set1_ps(SIGMOIDCOEFFICIENT);

	__m256 temp;
	const __m256* vOutput = (__m256*)neuronOutput;

	// 1 - ans
	temp = _mm256_sub_ps(ones, *vOutput);
	// (1-ans) * ans
	temp = _mm256_mul_ps(temp, *vOutput);
	// ans * coefficient
	temp = _mm256_mul_ps(temp, sigCoefficients);

#ifndef NDEBUG
	const float* _temp = (float*)&temp;
	assert(fastabs(_temp[0] - activationPrime(neuronOutput[0])) < 0.05f);
	assert(fastabs(_temp[1] - activationPrime(neuronOutput[1])) < 0.05f);
	assert(fastabs(_temp[2] - activationPrime(neuronOutput[2])) < 0.05f);
	assert(fastabs(_temp[3] - activationPrime(neuronOutput[3])) < 0.05f);
	assert(fastabs(_temp[4] - activationPrime(neuronOutput[4])) < 0.05f);
	assert(fastabs(_temp[5] - activationPrime(neuronOutput[5])) < 0.05f);
	assert(fastabs(_temp[6] - activationPrime(neuronOutput[6])) < 0.05f);
	assert(fastabs(_temp[7] - activationPrime(neuronOutput[7])) < 0.05f);
#endif

	// return ans
	_mm256_store_ps(result, temp);
};



void neuralNet::feedForward()
{
	for (layerIterator_t cLayer = net.begin()+1, lLayer = net.end(); cLayer != lLayer; ++cLayer)
	{
		feedForward_layer(cLayer);
	}
};

void neuralNet::feedForward_layer(layerIterator_t nLayer)
{
	constFloatIterator_t pActivations, cWeight, endWeight;
	__m256 vTotal, vSub0, vSub1;
	__m256 *vWeight, *vAct, *vEndWeight;

	// summate each neuron's contribution
	for (neuronIterator_t cNeuron = nLayer->begin(), end = nLayer->end(); 
		cNeuron != end; 
		++cNeuron)
	{
		// foreach [previous neuron, current weight], up to endWeight
		pActivations = activations.begin() + (nLayer - 1)->front().iNeuronIndex;
		cWeight = cNeuron->weightsBegin(*this);
		endWeight = cNeuron->weightsEnd(*this);

		// (first 15 neurons) (TODO: redesign preamble and remove assertions for multiple of 16 size widths in neuralNet.h!)

		// summate all neurons of previous layer: (remaining batches of 8 neurons)
		vWeight = (__m256*)&cWeight[0];
		vAct = (__m256*)&pActivations[0];

		vEndWeight = (__m256*)&endWeight[0];

		// initialize the activation of this neuron to its bias weight. The bias weight's neuron is always on:
		vTotal = _mm256_set_ps(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, *endWeight); // can this be made with an aligned load?

		do // Take advantage of SIMD instructions by doing 16 multiplies per iteration
		{
			/* 
			 * each neuron's contribution is:
			 * input[j] += weight[i,j] * activation[i]
			 */
			// multiply:
			vSub0 = _mm256_mul_ps(vWeight[0], vAct[0]);
			vSub1 = _mm256_mul_ps(vWeight[1], vAct[1]);

			// prefetch next values: (these don't appear to help, are the networks too small for this to matter?)
			//_mm_prefetch((char*)(vWeight0+4), _MM_HINT_T0);
			//_mm_prefetch((char*)(vAct0+4), _MM_HINT_T0);

			// add to accumulator:
			vTotal = _mm256_add_ps(vTotal, vSub0);
			vTotal = _mm256_add_ps(vTotal, vSub1);

			// increment pointers:
			vWeight += 2;
			vAct += 2;
		}
		while (vWeight != vEndWeight);

		//finalize: (combine all 4 accumulators)
		{
			vTotal = _mm256_hadd_ps(vTotal, vTotal);
			vTotal = _mm256_hadd_ps(vTotal, vTotal);
			__m128 vUpperTotal = _mm256_extractf128_ps(vTotal, 1);
			vUpperTotal = _mm_add_ps(vUpperTotal, _mm256_castps256_ps128(vTotal));

			// store the lowest float into cInput:
			_mm_store_ss(&activations[cNeuron->iNeuronIndex], vUpperTotal);
		}
	}

	// activate all neurons in this layer:
	float* cActivation = (&activations.front() + nLayer->front().iNeuronIndex);
	float* lActivation = (&activations.front() + nLayer->back().iNeuronIndex + 1);
	float* lVectorActivation = lActivation - ((lActivation - cActivation)&(ALIGN_SIZE-1)); // equivalent to mod ALIGN_SIZE

	// aligned activations:
	while (cActivation != lVectorActivation)
	{
		activation_approx_avx(cActivation, cActivation);
		cActivation += ALIGN_SIZE;
	};

	// postscript: (unaligned activations):
	{
		size_t dActivation = (lActivation - cActivation);
		switch(dActivation)
		{
		case 7:
			activation_approx(cActivation+6,cActivation+6);
		case 6:
			activation_approx(cActivation+5,cActivation+5);
		case 5:
			activation_approx(cActivation+4,cActivation+4);
		case 4:
			activation_approx_sse(cActivation+0,cActivation+0);
			break;
		case 3:
			activation_approx(cActivation+2, cActivation+2);
		case 2:
			activation_approx(cActivation+1, cActivation+1);
		case 1:
			activation_approx(cActivation+0, cActivation+0);
		case 0:
			break;
		}
	}
}; // endOf feedForward_layer





void neuralNet::randomizeWeights()
{
	// randomize each weight as a contiguous datastructure:
	BOOST_FOREACH(float& cWeight, weights)
	{
		cWeight = ( ( (float)rand() / (float)RAND_MAX) * 0.1f ) - 0.05f;
	}
};

void neuralNet::jitterWeights(float jitterMax)
{
	// randomize each weight as a contiguous datastructure:
	BOOST_FOREACH(float& cWeight, weights)
	{
		cWeight += ( deScale( (float)rand() / (float)RAND_MAX, jitterMax, 0.0f) * 2.0f ) - jitterMax;
	}
};





void neuralNet::clearInput()
{
	// zero all input neurons:
	std::memset(&activations.front(), 0, sizeof(float) * numInputs());
};

void neuralNet::clear()
{
	// clear everything:
	net.clear();
	activations.clear();
	weights.clear();
};





void neuralNet::output(std::ostream& oFile, bool printHeader) const
{
	/*
	 * Header data:
	 * PKNNT<VERSION> <floating point precision> <nnHeight> <name>\n
	 * <nnWidth 1> <nnWidth 2> .. <nnWidth n-1> <nnWidth n> \n
	 * <Weight 1,1,1> <Weight 1,1,2> .. <Weight 1,1,n-1> <Weight 1,1,n> \n
	 * <Weight 1,2,1> <Weight 1,2,2> .. <Weight 1,2,n-1> <Weight 1,2,n> \n
	 * ...
	 * <Weight L,m,1> <Weight L,m,2> .. <Weight L,m,n-1> <Weight L,m,n> \n
	 */

	// header:
	if (printHeader)
	{
		oFile << 
			header <<
			"\t" << 32 <<
			"\t" << net.size() <<
			"\t" << getName() <<
			"\n";
	}

	// widths:
	for (constLayerIterator_t lCurrent = net.begin(), lLast = net.end(); lCurrent != lLast; ++lCurrent)
	{
		oFile << lCurrent->size() << "\t";
	}
	oFile << "\n";

	// weights:
	for (constLayerIterator_t cLayer = net.begin()+1, lLayer = net.end(); cLayer != lLayer; ++cLayer)
	{
		if (printHeader) { oFile << "#iLayer " << (cLayer - net.begin()) << ":\n"; }
		for (constNeuronIterator_t cNeuron = cLayer->begin(), lNeuron = cLayer->end(); cNeuron != lNeuron; ++cNeuron)
		{
			if (cNeuron->numWeights() == 0) { continue; } 
			if (printHeader) { oFile << "#iNeuron " << (cNeuron - cLayer->begin()) << ":\n"; }
			// this loop includes the bias weight:
			for (size_t iWeight = 0, iSize = cNeuron->numWeights(); iWeight != iSize; ++iWeight)
			{
				uint32_t intWeight = ((uint32_t&)cNeuron->getWeight(*this, iWeight));
				oFile << std::hex << intWeight << "\t";
			}
			oFile << std::dec << "\n";
		}
	}
} // endOf output neuron





bool neuralNet::input(const std::vector<std::string>& lines, size_t& iLine)
{
	/*
	 * Header data:
	 * PKNNT<VERSION> <floating point precision> <nnHeight> <name>\n
	 * <nnWidth 1> <nnWidth 2> .. <nnWidth n-1> <nnWidth n> \n
	 * <Weight 1,1,1> <Weight 1,1,2> .. <Weight 1,1,n-1> <Weight 1,1,n> \n
	 * <Weight 1,2,1> <Weight 1,2,2> .. <Weight 1,2,n-1> <Weight 1,2,n> \n
	 * ...
	 * <Weight L,m,1> <Weight L,m,2> .. <Weight L,m,n-1> <Weight L,m,n> \n
	 */
	
	typedef union 
	{
		float fval;
		uint32_t ival;
	} intflt;

	// ensure empty neural entwork:
	clear();

	// are the enough lines in the input stream:
	if ((lines.size() - iLine) < 2U)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": unexpected end of input stream at line " << iLine << "!\n";
		return false; 
	}

	// compare neural network header:
	if (lines.at(iLine).compare(0, header.size(), header) != 0)
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": neural network stream has header of type \"" << lines.at(iLine).substr(0, header.size()) << 
			"\" (needs to be \"" << header 
			<< "\") and is incompatible with this program!\n";
		return false;
	}

	// read header:
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)4, (size_t)4)) { return false; }

		// make sure precision is 32:
		if (tokens.at(1).compare("32") != 0)
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": neural network stream has precision of \"" << tokens.at(1) << 
				"\" (needs to be 32) and is incompatible with this program!\n";
			return false;
		}
		// set name:
		setName(tokens.at(3).substr(0, 20));
	}

	size_t totalWeightedNeurons = 0;
	size_t totalWeights = 0;
	size_t readNeurons = 0;
	size_t readWeights = 0;

	// read widths:
	iLine++;
	{
		std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
		if (!INI::checkRangeB(tokens.size(), (size_t)1U, (size_t)MAXNETWORKLAYERS)) { return false; }
		net.reserve(tokens.size());

		size_t lastLayerNeurons = 0;
		for (size_t iLayer = 0; iLayer != tokens.size(); ++iLayer)
		{
			size_t layerWidth;
			if (!INI::setArgAndPrintError("network width", tokens.at(iLayer), layerWidth, iLine, iLayer)) { return false; }
			if (!INI::checkRangeB(tokens.size(), (size_t)1U, (size_t)MAXNETWORKWIDTH)) { return false; }

			net.push_back(std::vector<neuron>());

			// initialize to proper number of neurons, 
			//number of inputs equivalent to last layer number of neurons (fully connected)
			net.back().reserve(layerWidth);
			activations.reserve(activations.size() + layerWidth);
			for(size_t iNeuron = 0; iNeuron != layerWidth; ++iNeuron)
			{
				net.back().push_back(neuron(*this, lastLayerNeurons));
				totalWeights += net.back().back().numWeights();
			}

			assert(lastLayerNeurons % 16 == 0); // input and hidden layer neurons must be a multiple of 16
			if (iLayer > 0) { totalWeightedNeurons += layerWidth; }
			lastLayerNeurons = layerWidth;
		}
	}

	// determine we have at least as many lines as necessary to fill the array:
	if (!INI::checkRangeB(lines.size() - iLine, totalWeightedNeurons, SIZE_MAX)) { return false; }

	iLine++;
	for (size_t iLayer = 1; iLayer != net.size(); ++iLayer)
	{
		std::vector<neuron>& cLayer = net[iLayer];

		for (size_t iNeuron = 0; iNeuron != cLayer.size(); ++iNeuron)
		{
			neuron& cNeuron = cLayer[iNeuron];

			if (iLine >= lines.size())
			{
				std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
					": unexpected end of input stream at line " << iLine << "!\n";
				return false; 
			}

			if (lines.at(iLine)[0] == '#') { iNeuron--; iLine++; continue; }

			std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
			if (!INI::checkRangeB(tokens.size(), cNeuron.numWeights(), cNeuron.numWeights())) { return false; }

			// numWeights includes the bias weight
			for (size_t iWeight = 0; iWeight != cNeuron.numWeights(); ++iWeight)
			{
				intflt cWeight;
				std::istringstream tokenStream(tokens.at(iWeight));
				if (!(tokenStream >> std::hex >> cWeight.ival)) { INI::incorrectArgs("network weight", iLine, iWeight); return false; }
				
				if (boost::math::isnan(cWeight.fval)) { INI::incorrectArgs("network weight", iLine, iWeight); return false; }
				cNeuron.getWeight(*this, iWeight) = cWeight.fval;
			}

			readWeights += tokens.size();
			readNeurons++;
			iLine++;
		}
	}

	if ((totalWeightedNeurons != readNeurons) || (totalWeights != readWeights)) 
	{ 
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": Defined network topology does not match input!\n";
		return false; 
	} 
	return true;
}
