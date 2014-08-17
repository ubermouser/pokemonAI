#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>
#include <boost/array.hpp>
#include <boost/timer.hpp>

#include "../../inc/neuralNet.h"
#include "../../inc/backpropNet.h"

static boost::array< boost::array<float, 16>, 4 > xorTrials =
{{
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
}};

static boost::array< boost::array<float, 2>, 4 > xorResults =
{{
	{{ 0.0f, 1.0f }},
	{{ 1.0f, 0.0f }},
	{{ 1.0f, 0.0f }},
	{{ 0.0f, 1.0f }},
}};

static boost::array< boost::array<float, 16>, 4 > orTrials =
{{
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
}};

static boost::array< boost::array<float, 2>, 4 > orResults =
{{
	{{ 0.0f, 1.0f }},
	{{ 1.0f, 0.0f }},
	{{ 1.0f, 0.0f }},
	{{ 1.0f, 0.0f }},
}};

static boost::array< boost::array<float, 16>, 7 > hpTrials =
{{
	{{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }},
	{{ 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f }}
}};

static boost::array< boost::array<float, 2>, 7 > hpResults =
{{
	{{ 1.0f, 0.0f }},
	{{ 0.0f, 1.0f }},
	{{ 0.5f, 0.5f }},
	{{ 0.5f, 0.5f }},
	{{ 0.5f, 0.5f }},
	{{ 0.75f, 0.25f }},
	{{ 0.25f, 0.75f }}
}};

template<size_t numTrials, size_t inputWidth, size_t outputWidth>
bool trial(const backpropSettings& bSettings, const boost::array< boost::array<float, inputWidth>, numTrials >& cTrials, const boost::array< boost::array<float, outputWidth>, numTrials >& cResults)
{
	size_t midWidth = (inputWidth + outputWidth);
	while ((midWidth & 15) != 0) { ++midWidth; };
	boost::array<size_t, 3> layers = {{inputWidth , midWidth , outputWidth}};

	std::cout << "dimensions= " << inputWidth << "," << midWidth << "," << outputWidth << "\nnumTrials= " << cTrials.size() << "\n";

	//boost::array<size_t, 2> layers = {{16 , 2}};
	neuralNet cNet(layers.begin(), layers.end());

	backpropNet bpNet(cNet, bSettings);
	bpNet.randomizeWeights();

	double _MSE = 1.0;
	double MSE = 0.0;
	double numTrials = 0.0;
	boost::timer cTimer;
	for (size_t iEpoch = 0; iEpoch < 3000; ++iEpoch)
	{
		// determine an order that we intend to visit the trials:
		std::vector<size_t> order(cTrials.size());
		for (size_t iTurn = 0; iTurn != order.size(); ++iTurn) { order[iTurn] = iTurn; }
		std::random_shuffle(order.begin(), order.end());

		// jitter:
		if ((iEpoch % 1000) == 0)
		{
			bpNet.jitterNetwork();
		}

		// perform trials:
		for (size_t iNTrial = 0; iNTrial != order.size(); ++iNTrial)
		{
			size_t iTrial = order[iNTrial];

			bpNet.getNeuralNet().feedForward(cTrials[iTrial].begin());
			MSE += bpNet.backPropagate(cResults[iTrial].begin());
			assert(MSE == MSE);
			numTrials += 1.0;
			bpNet.updateWeights();
		}


		// debug:
		if ((iEpoch+1) % 100 == 0)
		{
			//rankedNet.updateWeights();
			_MSE = MSE/numTrials;
			std::cout << std::setw(4) << iEpoch << " " << _MSE << "\n";
			MSE = 0.0;
			numTrials = 0.0;
		}
	}

	std::cout << "test " << ((_MSE<0.05)?"passed":"FAILED") << "! Elapsed time = " << cTimer.elapsed() << "\n";
	return _MSE<0.05;
};

int verbose = 10;

int main()
{
	backpropSettings bSettings(0.25f, 0.2f, 0.3f);
	srand(5);

	std::cout << "vars:" <<
		"\n\tlearnRate = " << std::setw(12) << bSettings.learningRate <<
		"\n\tmomentum  = " << std::setw(12) << bSettings.momentum <<
		"\n\tjitter    = " << std::setw(12) << bSettings.jitterMax <<
		"\n";

	bool result = true;
	std::cout << "\norTrial:\n\n";
	result &= trial(bSettings, orTrials, orResults);
	std::cout << "\nxorTrial:\n\n";
	result &= trial(bSettings, xorTrials, xorResults);
	std::cout << "\nhpTrial:\n\n";
	result &= trial(bSettings, hpTrials,hpResults);

	return result;
}
