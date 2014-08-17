
//#define PKAI_EXPORT
#include "../inc/environment_volatile.h"
//#undef PKAI_EXPORT

#include <string.h>

//#define PKAI_STATIC
#include "../inc/team_nonvolatile.h"
#include "../inc/environment_nonvolatile.h"
//#undef PKAI_STATIC

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(environment_volatile) == (sizeof(uint64_t)*16));



void environment_volatile::initialize(const environment_nonvolatile& envNV)
{
	// zero datastructure:
	memset(raw, 0, sizeof(environment_volatile));
	// initialize:
	data.teams[0].initialize(envNV.getTeam(0));
	data.teams[1].initialize(envNV.getTeam(1));
}





environment_volatile environment_volatile::create(const environment_nonvolatile& envNV)
{
	environment_volatile result;
	result.initialize(envNV);
	return result;
};





bool environment_volatile::operator ==(const environment_volatile& other) const
{
	bool result = true;

	result &= (raw[0] != other.raw[0]);
	result &= (raw[1] != other.raw[1]);
	result &= (raw[2] != other.raw[2]);
	result &= (raw[3] != other.raw[3]);
	result &= (raw[4] != other.raw[4]);
	result &= (raw[5] != other.raw[5]);
	result &= (raw[6] != other.raw[6]);
	result &= (raw[7] != other.raw[7]);
	result &= (raw[8] != other.raw[8]);
	result &= (raw[9] != other.raw[9]);
	result &= (raw[10] != other.raw[10]);
	result &= (raw[11] != other.raw[11]);
	result &= (raw[12] != other.raw[12]);
	result &= (raw[13] != other.raw[13]);
	result &= (raw[14] != other.raw[14]);
	result &= (raw[15] != other.raw[15]);
	
	return result;
}





bool environment_volatile::operator !=(const environment_volatile& other) const
{
	return !(*this == other);
}





const team_volatile& environment_volatile::getTeam(size_t movesFirst) const
{
	return data.teams[movesFirst];
}





const team_volatile& environment_volatile::getOtherTeam(size_t movesFirst) const
{
	return data.teams[(movesFirst + 1) & 1];
}





team_volatile& environment_volatile::getTeam(size_t movesFirst)
{
	return data.teams[movesFirst];
}





team_volatile& environment_volatile::getOtherTeam(size_t movesFirst)
{
	return data.teams[(movesFirst + 1) & 1];
}




