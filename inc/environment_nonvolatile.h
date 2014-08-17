#ifndef ENVIRONMENT_NONVOLATILE_H
#define	ENVIRONMENT_NONVOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <boost/array.hpp>

#include "../inc/team_nonvolatile.h"

class PKAISHARED environment_nonvolatile
{
private:
	boost::array<team_nonvolatile, 2> teams;

public:

	environment_nonvolatile()
		: teams()
	{
	};

	environment_nonvolatile(const team_nonvolatile& _teamA, const team_nonvolatile& _teamB, bool init = false)
	{
		teams[0] = _teamA;
		teams[1] = _teamB;
		if (init) { initialize(); };
	};

	environment_nonvolatile(const environment_nonvolatile& other)
		: teams(other.teams)
	{
	};

	team_nonvolatile& getTeam(size_t movesFirst)
	{
		return teams[movesFirst];
	};

	team_nonvolatile& getOtherTeam(size_t movesFirst)
	{

		return teams[(movesFirst+1)&1];
	};

	friend union environment_volatile;

public:

	void initialize();

	void uninitialize();

	const team_nonvolatile& getTeam(size_t movesFirst) const
	{
		return teams[movesFirst];
	};

	const team_nonvolatile& getOtherTeam(size_t movesFirst) const
	{
		return teams[(movesFirst+1)&1];
	};

	void setTeam(size_t iTeam, const team_nonvolatile& cTeam, bool init = false);

};

#endif /* ENVIRONMENT_NONVOLATILE_H */
