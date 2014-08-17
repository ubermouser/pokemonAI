#ifndef GENETIC_H
#define GENETIC_H

#include "../inc/pkai.h"
#include <stdint.h>
#include <limits>
#include <math.h>

#include "../inc/fp_compare.h"
#include "../inc/ranked.h"
#include "../inc/ranked_team.h"
#include "../inc/trueSkill.h"




// sorter classes and functions:
class sortByMatchQuality
{
private:
	const trueSkillSettings& tSettings;
	const trueSkillTeam& cTeam;
public:
	sortByMatchQuality(const trueSkillTeam& _cTeam, const trueSkillSettings& _tSettings = trueSkillSettings::defaultSettings)
		: tSettings(_tSettings),
		cTeam(_cTeam)
	{
	};

	fpType getValue(const trueSkillTeam& oTeam) const
	{
		// don't attempt to find match against an identical team:
		if (*(cTeam.baseTeam) == *(oTeam.baseTeam)) { return std::numeric_limits<fpType>::quiet_NaN(); }

		// don't attempt to find match against an identical neural network, unless the networks are the base evaluator:
		fpType multiplier = 1.0;
		if (*(cTeam.baseEvaluator) == *(oTeam.baseEvaluator))
		{
			// seeded evaluators may play themselves (at reduced frequency):
			/*if (cTeam.baseEvaluator->getHash() == ranked::defaultHash) { multiplier = 0.25; }
			// but neuralNet evaluators may not
			else { return std::numeric_limits<fpType>::quiet_NaN(); }*/
			multiplier = 0.25;
		}

		// if either of these teams have the same pokemon, do not allow the same pokemon to fight
		for (size_t iOTeammate = 0, iOSize = oTeam.baseTeam->team.getNumTeammates(); iOTeammate != iOSize; ++iOTeammate)
		{
			for (size_t iCTeammate = 0, iCSize = cTeam.baseTeam->team.getNumTeammates(); iCTeammate != iCSize; ++iCTeammate)
			{
				if (cTeam.baseTeam->getTeammateHash(iCTeammate) == oTeam.baseTeam->getTeammateHash(iOTeammate)) 
				{ 
					return std::numeric_limits<fpType>::quiet_NaN();
				}
			}
		}

		// determine match quality
		fpType cQuality = trueSkill::matchQuality(cTeam, oTeam, tSettings);
		cQuality = cQuality * cQuality * multiplier;
		return mostlyLT(cQuality, 0.0001)?0.0:cQuality;
	};
};

class sortByVariability
{
public:
	static fpType getValue (const ranked& cTeam)
	{
		return cTeam.getSkill().getStdDev() * cTeam.getSkill().getStdDev();
	};

	static fpType getValue (const ranked* const cTeam)
	{
		return cTeam->getSkill().getStdDev() * cTeam->getSkill().getStdDev();
	};
};

class sortByMean
{
public:

	static fpType getValue (const ranked& oTeam)
	{
		fpType oMean = oTeam.getSkill().getMean();
		return 1.0 + (mostlyGT(oMean, 0.0)?sqrt(oMean):0.0);
	};
};

class sortByInverseMean
{
	fpType maxMean;
public:
	sortByInverseMean(fpType _maxMean)
		: maxMean(_maxMean)
	{
	};

	fpType getValue (const ranked& oTeam) const
	{
		// the element with the highest mean has a 0% chance of being selected
		fpType oMean = oTeam.getSkill().getMean();
		return maxMean - oMean;
	};
};

class sortByMean_noDuplicates
{
private:
	const ranked_team& cTeam;
	bool partial;
public:
	sortByMean_noDuplicates(const ranked_team& _cTeam, bool _partial = false)
		: cTeam(_cTeam),
		partial(_partial)
	{
	};

	fpType getValue (const ranked_team& oTeam) const
	{
		// compare team hash - don't allow duplicate
		if (cTeam == oTeam) { return std::numeric_limits<fpType>::quiet_NaN(); }

		if (!partial)
		{
			// compare teammates themselves: don't allow teams that cannot be added
			for (size_t iTeammate = 0; iTeammate != oTeam.team.getNumTeammates(); ++iTeammate)
			{
				if (!cTeam.team.isLegalAdd(oTeam.team.teammate(iTeammate))) 
				{ 
					return std::numeric_limits<fpType>::quiet_NaN();
				}
			}
		}
		// finally, weight fitness by the team's mean score
		fpType oMean = oTeam.getSkill().getMean();
		return 1.0 + (mostlyGT(oMean, 0.0)?sqrt(oMean):0.0);
	};
};

class sortByRank
{
private:
	const ranked_team& cTeam;
	fpType minRank;
	fpType maxRank;
public:
	sortByRank(const ranked_team& _cTeam, fpType _minRank, fpType _maxRank)
		: cTeam(_cTeam),
		minRank(_minRank),
		maxRank(_maxRank)
	{
	}
	fpType getValue (const ranked_team& oTeam) const
	{
		if (cTeam == oTeam) { return std::numeric_limits<fpType>::quiet_NaN(); }
		return 0.1 + scale(cTeam.getSkill().getRank(), maxRank, minRank);
	};
};

#endif /* GENETIC_H */
