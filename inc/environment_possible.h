/* 
 * File:   possible_environment.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:25 PM
 */

#ifndef POSSIBLE_ENVIRONMENT_H
#define	POSSIBLE_ENVIRONMENT_H

#include "../inc/pkai.h"

#include <stdint.h>

#include "../src/fixedpoint/fixed_class.h"
#include "../inc/environment_volatile.h"

typedef fixedpoint::fixed_point<30> fixType;

union PKAISHARED environment_possible
{
	struct
	{
		/*
		 * the environment this possible_environment represents
		 */
		environment_volatile env;

		/* the hashed value of this completed environment_possible. Environment should NOT BE CHANGED once hashed! */
		uint64_t hash;

		/*
		 * the probability that this environment will occur 
		 * given the previous environment's probability is 1, and
		 * the probability of the actions that occured to create this
		 * environment are 1
		 */
		fixType probability;

		/*
		 * what type of action occured to create this environment from
		 * the previous environment?
		 * 
		 * if bit n is set to 1:
		 * 0 - team a primary effect = hit
		 * 1 - team a critical hit = yes
		 * 2 - team a secondary effect = hit
		 * 3 - a status effect prevented team a from acting
		 * 4 - team a pokemon recently switched out
		 * 5 - team a dead pokemon recently got a free switch
		 * 
		 * 8 - team b primary effect = hit
		 * 9 - team b critical hit = yes
		 * 10 - team b secondary effect = hit
		 * 11 - a status effect prevented the team b from acting
		 * 12 - team b pokemon recently switched out
		 * 13 - team b dead pokemon recently got a free switch out

		 * 7 - this environmentPossible has been pruned due to duplicate status
		 * 15 - this environmentPossible has been merged with a duplicate environment
		 * 
		 */
		uint32_t envBitset;
	} data;
	uint64_t raw[18];

	static environment_possible create(const environment_volatile& source, bool doHash = false);

	/* Is the probability of this entity occuring less than the probability of 
	 the other entity occuring?*/
	bool operator<(const environment_possible& other) const;

	const environment_volatile& getEnv() const { return data.env; };

	environment_volatile& getEnv() { return data.env; }

	const fixType& getProbability() const { return data.probability; };

	fixType& getProbability() { return data.probability; };

	const uint32_t& getBitmask() const { return data.envBitset; };

	uint32_t& getBitmask() { return data.envBitset; };

	const uint64_t& getHash() const { return data.hash; };

	void setHit(size_t iTeam)
	{
		data.envBitset = data.envBitset | (0x1<<(0 + iTeam*8));
	};

	void setCrit(size_t iTeam)
	{
		data.envBitset = data.envBitset | (0x1<<(1 + iTeam*8));
	};

	void setSecondary(size_t iTeam)
	{
		data.envBitset = data.envBitset | (0x1<<(2 + iTeam*8));
	};

	void setBlocked(size_t iTeam)
	{
		data.envBitset = data.envBitset | (0x1<<(3 + iTeam*8));
	};

	void setSwitched(size_t iTeam)
	{
		data.envBitset = data.envBitset | (0x1<<(4 + iTeam*8));
	};

	void setFreeMove(size_t iTeam)
	{
		data.envBitset = data.envBitset | (0x1<<(5 + iTeam*8));
	};

	void setWaited(size_t iTeam)
	{
		data.envBitset = data.envBitset | (0x1<<(6 + iTeam*8));
	}

	void setMerged()
	{
		data.envBitset = data.envBitset | (0x1<<(15));
	};

	void setPruned()
	{
		data.envBitset = data.envBitset | (0x1<<(7));
	};

	/* has iTeam hit this round? */
	bool hasHit(size_t iTeam) const
	{
		return (0x1<<(0 + iTeam*8) & data.envBitset) > 0;
	};

	/* has iTeam critical hit this round? */
	bool hasCrit(size_t iTeam) const
	{
		return (0x1<<(1 + iTeam*8) & data.envBitset) > 0;
	};

	/* has iTeam used a secondary effect this round? */
	bool hasSecondary(size_t iTeam) const
	{
		return (0x1<<(2 + iTeam*8) & data.envBitset) > 0;
	};

	/* was iteam's action blocked this round? */
	bool wasBlocked(size_t iTeam) const
	{
		return (0x1<<(3 + iTeam*8) & data.envBitset) > 0;
	};

	/* has iTeam switched this round? */
	bool hasSwitched(size_t iTeam) const
	{
		return (0x1<<(4 + iTeam*8) & data.envBitset) > 0;
	};
	
	/* has iTeam used a free move this round? */
	bool hasFreeMove(size_t iTeam) const
	{
		return (0x1<<(5 + iTeam*8) & data.envBitset) > 0;
	};

	/* has iTeam used waited this round? */
	bool hasWaited(size_t iTeam) const
	{
		return (0x1<<(6 + iTeam*8) & data.envBitset) > 0;
	};

	bool isMerged() const
	{
		return (0x1<<(15) & data.envBitset) > 0;
	};

	bool isPruned() const
	{
		return (0x1<<(7) & data.envBitset) > 0;
	};

	void generateHash();
};

class PKAISHARED envP_print
{
private:
	const environment_nonvolatile& envNV;
	const environment_possible& envP;
	size_t agentTeam;

public:
	envP_print(const environment_nonvolatile& _envNV, const environment_possible& _envP, size_t _agentTeam = TEAM_A)
		: envNV(_envNV),
		envP(_envP),
		agentTeam(_agentTeam)
	{
	};

	friend PKAISHARED std::ostream& operator <<(std::ostream& os, const envP_print& environment);
};

PKAISHARED std::ostream& operator <<(std::ostream& os, const envP_print& environment);

#endif	/* POSSIBLE_ENVIRONMENT_H */

