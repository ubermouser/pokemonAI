#include <stddef.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <assert.h>
#include <boost/array.hpp>
#include <boost/static_assert.hpp>

#include "../../inc/experienceNet.h"

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

static boost::array< boost::array<float, 1>, 8> simpleTrials =
{{
	{{ 0.0  }},
	{{ 0.05  }},
	{{ 0.1 }},
	{{ 0.125 }},
	{{ 1.0 }},
	{{ 0.95 }},
	{{ 0.9 }},
	{{ 0.875}}
}};

static boost::array< boost::array<float, 1>, 8> noEntropyTrials =
{{
	{{ 0.0  }},
	{{ 0.0  }},
	{{ 0.0 }},
	{{ 0.0 }},
	{{ 0.0 }},
	{{ 0.0 }},
	{{ 0.0 }},
	{{ 0.0 }}
}};

static boost::array< boost::array<float, 1>, 8> fullEntropyTrials =
{{
	{{ 0.0  }},
	{{ 1.0  }},
	{{ 0.0 }},
	{{ 1.0 }},
	{{ 0.0 }},
	{{ 1.0 }},
	{{ 0.0 }},
	{{ 1.0 }}
}};



template<size_t numTrials, size_t trialSize>
bool networkExperienceTest(const boost::array< boost::array< float, trialSize >, numTrials >& cTrials, const experienceNetSettings& cSettings)
{
BOOST_STATIC_ASSERT(numTrials > 0 && trialSize > 0);
	boost::array<float, trialSize> cResult;
	experienceNet testNet(cResult.size(), cSettings);

	for (size_t iTrial = 0; iTrial != cTrials.size(); ++iTrial)
	{
		const boost::array< float, trialSize >& cTrial = cTrials[iTrial];
		testNet.addExperience(cTrial.begin(), 1.0);
	}

	for (size_t iTrial = 0; iTrial != cTrials.size(); ++iTrial)
	{
		std::cout
			<< std::setw(15) << cTrials[iTrial][0] << " : "
			<< std::setw(15) << testNet.getExperience(cTrials[iTrial].begin()) << "\n";
	}
	std::cout << 
		   "\nmax     =" << std::setw(15) << testNet.maximum()
		<< "\nmin     =" << std::setw(15) << testNet.minimum()
		<< "\nentropy =" << std::setw(15) << testNet.entropy()
		<< "\n\n";
	float dVal = 1.0f / (cSettings.numTaps);
	float cVal = 0.0f;
	for (size_t iVal = 0; iVal != (cSettings.numTaps+1); ++iVal)
	{
		cResult.assign(cVal);
		std::cout 
			<< std::setw(15) << cVal << " : "
			<< std::setw(15) << testNet.getExperience(cResult.begin()) << "\n";
		cVal += dVal;
	}

	{
		experienceNet copiedNet = testNet;
		std::cout << "assignment operator copy " << ((copiedNet.maximum() != testNet.maximum())?"FAILED":"PASSED") << "!\n";
	}
	{
		experienceNet initNet(testNet);
		std::cout << "copy constructor copy " << ((initNet.maximum() != testNet.maximum())?"FAILED":"PASSED") << "!\n";
	}
	return true;
};

void main()
{
	std::cout << "simpleTrials:\n\n";
	networkExperienceTest(simpleTrials, experienceNetSettings(8, 1.0f / 12.0f, 0.95f, EXPERIENCENET_RECENCY));
	std::cout << "hpTrials:\n\n";
	networkExperienceTest(hpTrials, experienceNetSettings(8, 1.0f / 12.0f, 0.95f, EXPERIENCENET_HISTOGRAM));
	std::cout << "noEntropyTrials:\n\n";
	networkExperienceTest(noEntropyTrials, experienceNetSettings(8, 1.0f / 36.0f, 1.0f, EXPERIENCENET_HISTOGRAM));
	std::cout << "fullEntropyTrials:\n\n";
	networkExperienceTest(fullEntropyTrials, experienceNetSettings(2, 1.0f / 36.0f, 1.0f, EXPERIENCENET_HISTOGRAM));
};
