#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>
#include <boost/array.hpp>
#include <boost/foreach.hpp>

#include <math.h>

#include "../../inc/neuralNet.h"
#include "../../inc/temporalpropNet.h"
#include "../../inc/backpropNet.h"

static boost::array< boost::array<float, 16>, 13 > hpTrials =
{{
	// forward:
	{{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f,		0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f,		0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f,		0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }}
}};

static boost::array< boost::array<float, 2>, 13 > hpResults =
{{
	{{ 0.5f, 0.5f }},
	{{ 0.5f, 0.5f }},
	{{ 0.476f, 0.524f }},
	{{ 0.473f, 0.527f }},
	{{ 0.470f, 0.530f }},
	{{ 0.4375f, 0.5625f }},
	{{ 0.428f, 0.572f }},
	{{ 0.416f, 0.584f }},
	{{ 0.363f, 0.637f }},
	{{ 0.333f, 0.667f }},
	{{ 0.285f, 0.715f }},
	{{ 0.166f, 0.834f }},
	{{ 0.0f, 1.0f }}
}};

static boost::array< boost::array<float, 16>, 13 > revHpTrials =
{{
	// forward:
	{{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f,		0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,		0.0f, 0.0f, 0.0f, 0.0f }}
}};

static boost::array< boost::array<float, 2>, 13 > revHpResults =
{{
	{{ 0.5f, 0.5f }},
	{{ 0.5f, 0.5f }},
	{{ 0.524f, 0.476f }},
	{{ 0.527f, 0.473f }},
	{{ 0.530f, 0.470f }},
	{{ 0.5625f, 0.4375f }},
	{{ 0.572f, 0.428f }},
	{{ 0.584f, 0.416f }},
	{{ 0.637f, 0.363f }},
	{{ 0.667f, 0.333f }},
	{{ 0.715f, 0.285f }},
	{{ 0.834f, 0.166f }},
	{{ 1.0f, 0.0f }}
}};

static boost::array<float, 1> nullResult = {{ 0.0f }};

bool outputPrediction(neuralNet& cNet)
{
	double MSE = 0.0;
	std::vector<float> lastOutput(cNet.numOutputs());
	//output prediction:
	size_t numEpochs = 2;
	for (size_t iEpoch = 0; iEpoch < numEpochs; ++iEpoch)
	{
		bool iTeam = /*false;*/iEpoch%2==1;
		boost::array< boost::array<float, 16>, 13 >& cTrials = (iTeam)?hpTrials:revHpTrials;
		boost::array< boost::array<float, 2>, 13 >& cResults = (iTeam)?hpResults:revHpResults;
		double _MSE = 0.0;

		for (size_t iTrial = 0; iTrial != cTrials.size(); ++iTrial)
		{
			cNet.feedForward(cTrials[iTrial].begin());
			floatIterator_t nOutput = cNet.outputBegin();
			boost::array<float, 2>::iterator trialOutput = cResults.back().begin();
			BOOST_FOREACH(float& cOutput, lastOutput)
			{
				_MSE += pow(*nOutput - *trialOutput,2);
				cOutput = *nOutput;
				++nOutput;
				++trialOutput;
			}
			std::cout <<
				"Trial " << std::setw(2) << iTrial << 
				" expected ";
			BOOST_FOREACH(float& cResult, cResults[iTrial])
			{
				std::cout << std::setw(12) << cResult << " ";
			}
			std::cout << "\tgot ";
			BOOST_FOREACH(float& cOutput, lastOutput)
			{
				std::cout << std::setw(12) << cOutput << " ";
			}
			std::cout << "\n";
		}
		_MSE /= cTrials.size();
		MSE += _MSE;
		std::cout << "\n";
	}
	MSE /= numEpochs;
	std::cout << "Final MSE=" << std::setw(12) << MSE << "\n";
	return (MSE<0.05);
};

bool lambdaTest(const temporalpropSettings& tSettings)
{
	boost::array<size_t, 3> layers = {{16 , 16 , 1}};
	neuralNet cNet(layers.begin(), layers.end());

	temporalpropNet tNet(cNet, tSettings);
	tNet.randomizeWeights();

	std::vector<float> lastOutput(tNet.getNeuralNet().numOutputs());
	double _MSE = 1.0;
	double MSE = 0.0;
	double numTrials = 0.0;
	for (size_t iEpoch = 0; iEpoch < 3000; ++iEpoch)
	{
		bool iTeam = /*false;*/iEpoch%2==1;
		boost::array< boost::array<float, 16>, 13 >& cTrials = (iTeam)?hpTrials:revHpTrials;
		boost::array< boost::array<float, 2>, 13 >& cResults = (iTeam)?hpResults:revHpResults;

		if ((iEpoch%1000)==0 && (iEpoch != 0)) { tNet.jitterNetwork(); }

		// determine an order that we intend to visit the trials:
		std::vector<size_t> order(cTrials.size());
		for (size_t iTurn = 0; iTurn != cTrials.size(); ++iTurn) { order[iTurn] = cTrials.size() - iTurn - 1; } // backward
		//for (size_t iTurn = 0; iTurn != cTrials.size(); ++iTurn) { order[iTurn] = iTurn; } // forward
		//std::random_shuffle(order.begin(), order.end());

		tNet.getNeuralNet().clearInput();

		for (size_t iNTrial = 0; iNTrial != order.size(); ++iNTrial)
		{
			size_t iTrial = order[iNTrial];

			// terminal turn first:
			if (iNTrial == 0) // backward
			//if ((iNTrial + 1) == cTrials.size()) // forward
			{
				tNet.getNeuralNet().feedForward(cTrials[iTrial].begin());
				// update eligibility traces based on lambda
				tNet.updateEligibilities();

				// backpropagate a terminal result:
				MSE += tNet.backPropagate(cResults[iTrial].begin(), false);
			}
			// non-terminal turn:
			else if ((iNTrial + 1) != cTrials.size()) // backward
			//else if (iNTrial > 0) // forward
			{
				// calculate previous node output:
				const boost::array<float, 16>& pTrial = cTrials[(size_t)std::max(0, (int32_t)iTrial-1)];
				// determine previous node output
				tNet.getNeuralNet().feedForward(pTrial.begin());
				tNet.updateEligibilities();
				
				// save as previous trial output:
				floatIterator_t nOutput = tNet.getNeuralNet().outputBegin();
				BOOST_FOREACH(float& cOutput, lastOutput)
				{
					cOutput = *nOutput++;
				}

				// feed forward with cEnv and envNV feature vector, but do not interpret output:
				tNet.getNeuralNet().feedForward(cTrials[iTrial].begin());
				// update eligibility traces based on lambda
				

				// propagate the current evaluation as the last evaluation's error:
				MSE += tNet.temporalPropagate(nullResult.begin(), lastOutput.begin());
			}
			/*else // iNTurn = 0 .. forward case only
			{
				bNet.getNeuralNet().feedForward(cTrials[iTrial].begin());
				// update eligibility traces based on lambda
				bNet.updateEligibilities();
			}*/

			tNet.updateWeights();
			numTrials += (double)cResults[iTrial].size();
		}
		tNet.zeroEligibilities();

		// debug:
		if ((iEpoch+1) % 100 == 0)
		{
			_MSE = MSE/numTrials;
			std::cout << std::setw(4) << iEpoch << " " << _MSE << "\n";
			MSE = 0.0;
			numTrials = 0.0;
		}
	}

	bool result = outputPrediction(tNet.getNeuralNet());
	result &= (_MSE < 0.1);

	if (result) 
	{ 
		std::cout << "lambda test passed!\n";
	}
	else
	{
		std::cout << "lambda test FAILED!\n";
	}
	return result;
};

bool qTest(const temporalpropSettings& tSettings)
{
	boost::array<size_t, 3> layers = {{16 , 16 , 1}};
	neuralNet cNet(layers.begin(), layers.end());

	backpropNet bNet(cNet, tSettings);
	bNet.randomizeWeights();

	std::vector<float> lastOutput(bNet.getNeuralNet().numOutputs());
	double _MSE = 1.0;
	double MSE = 0.0;
	double numTrials = 0.0;
	for (size_t iEpoch = 0; iEpoch < 3000; ++iEpoch)
	{
		bool iTeam = /*false;*/iEpoch%2==1;
		boost::array< boost::array<float, 16>, 13 >& cTrials = (iTeam)?hpTrials:revHpTrials;
		boost::array< boost::array<float, 2>, 13 >& cResults = (iTeam)?hpResults:revHpResults;

		if ((iEpoch%1000)==0 && (iEpoch != 0)) { bNet.jitterNetwork(); }

		// determine an order that we intend to visit the trials:
		std::vector<size_t> order(cTrials.size());
		for (size_t iTurn = 0; iTurn != cTrials.size(); ++iTurn) { order[iTurn] = cTrials.size() - iTurn - 1; } // backward
		//for (size_t iTurn = 0; iTurn != cTrials.size(); ++iTurn) { order[iTurn] = iTurn; } // forward
		//std::random_shuffle(order.begin(), order.end());

		bNet.getNeuralNet().clearInput();

		for (size_t iNTrial = 0; iNTrial != order.size(); ++iNTrial)
		{
			size_t iTrial = order[iNTrial];

			// terminal turn first:
			if (iNTrial == 0) // backward
			//if ((iNTrial + 1) == cTrials.size()) // forward
			{
				bNet.getNeuralNet().feedForward(cTrials[iTrial].begin());
				// update eligibility traces based on lambda

				// backpropagate a terminal result:
				MSE += bNet.backPropagate(cResults[iTrial].begin());
			}
			// non-terminal turn:
			else if ((iNTrial + 1) != cTrials.size()) // backward
			//else if (iNTrial > 0) // forward
			{
				// feed forward with cEnv and envNV feature vector, but do not interpret output:
				bNet.getNeuralNet().feedForward(cTrials[iTrial].begin());
				
				// save as previous trial output:
				floatIterator_t nOutput = bNet.getNeuralNet().outputBegin();
				BOOST_FOREACH(float& cOutput, lastOutput)
				{
					cOutput = *nOutput++;
				}
				
				// calculate previous node output:
				const boost::array<float, 16>& pTrial = cTrials[(size_t)std::max(0, (int32_t)iTrial-1)];
				// determine previous node output
				bNet.getNeuralNet().feedForward(pTrial.begin());

				// propagate the current evaluation as the last evaluation's error:
				MSE += bNet.backPropagate(lastOutput.begin());
			}
			/*else // iNTurn = 0 .. forward case only
			{
				bNet.getNeuralNet().feedForward(cTrials[iTrial].begin());
				// update eligibility traces based on lambda
				bNet.updateEligibilities();
			}*/

			bNet.updateWeights();
			numTrials += (double)cResults[iTrial].size();
		}

		// debug:
		if ((iEpoch+1) % 100 == 0)
		{
			_MSE = MSE/numTrials;
			std::cout << std::setw(4) << iEpoch << " " << _MSE << "\n";
			MSE = 0.0;
			numTrials = 0.0;
		}
	}

	bool result = outputPrediction(bNet.getNeuralNet());
	result &= (_MSE < 0.05);

	if (result) 
	{ 
		std::cout << "qlearning test passed!\n";
	}
	else
	{
		std::cout << "qlearning test FAILED!\n";
	}
	return result;
};

int main()
{
	temporalpropSettings tSettings(0.225f, 0.2f, 0.3f, 0.8f, 1.0f);
	srand(5);

	std::cout << "vars:" <<
		"\n\tlearnRate = " << std::setw(12) << tSettings.learningRate <<
		"\n\tmomentum  = " << std::setw(12) << tSettings.momentum <<
		"\n\tjitter    = " << std::setw(12) << tSettings.jitterMax <<
		"\n\tlambda    = " << std::setw(12) << tSettings.lambda <<
		"\n\tgamma     = " << std::setw(12) << tSettings.gamma <<
		"\n";

	bool result = true;
	std::cout << "\n\nQLearning test:\n";
	result &= qTest(tSettings);
	std::cout << "\n\nTDLearning test:\n";
	result &= lambdaTest(tSettings);

	return result;
};
