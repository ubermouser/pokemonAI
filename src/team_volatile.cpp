
//#define PKAI_EXPORT
#include "../inc/team_volatile.h"
//#undef PKAI_EXPORT

#include "../inc/pokemon_nonvolatile.h"
#include "../inc/team_nonvolatile.h"

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(team_volatile) == (sizeof(uint64_t)*8));
BOOST_STATIC_ASSERT(sizeof(volatileStatus) == (sizeof(uint32_t)*3));
BOOST_STATIC_ASSERT(sizeof(nonvolatileStatus) == (sizeof(uint32_t)*1));





bool team_volatile::operator ==(const team_volatile& other) const
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
	
	return result;
}





bool team_volatile::operator !=(const team_volatile& other) const
{
	return !(*this == other);
}





void team_volatile::initialize(const team_nonvolatile& nv)
{
	// status and all other variables have been zeroed from a different context 
	for (size_t iTeammate = 0, _numTeammates = nv.getNumTeammates(); iTeammate != _numTeammates; ++iTeammate)
	{
		teammate(iTeammate).initialize(nv.teammate(iTeammate));
	}
}





int32_t team_volatile::cGetBoost(size_t type) const
{
	switch(type)
	{
	case FV_ATTACK:
		return data.status.data.cTeammate.data.boosts.data.B_ATK;
	case FV_DEFENSE:
		return data.status.data.cTeammate.data.boosts.data.B_DEF;
	case FV_SPATTACK:
		return data.status.data.cTeammate.data.boosts.data.B_SPA;
	case FV_SPDEFENSE:
		return data.status.data.cTeammate.data.boosts.data.B_SPD;
	case FV_SPEED:
		return data.status.data.cTeammate.data.boosts.data.B_SPE;
	case FV_ACCURACY:
		return data.status.data.cTeammate.data.boosts.data.B_ACC;
	case FV_EVASION:
		return data.status.data.cTeammate.data.boosts.data.B_EVA;
	case FV_CRITICALHIT:
		return data.status.data.cTeammate.data.boosts.data.B_CHT;
	default:
	case FV_HITPOINTS:
		return 0;
	}
};





void team_volatile::cSetBoost(size_t type, int32_t value)
{
	switch(type)
	{
	case FV_ATTACK:
		data.status.data.cTeammate.data.boosts.data.B_ATK = value; return;
	case FV_DEFENSE:
		data.status.data.cTeammate.data.boosts.data.B_DEF = value; return;
	case FV_SPATTACK:
		data.status.data.cTeammate.data.boosts.data.B_SPA = value; return;
	case FV_SPDEFENSE:
		data.status.data.cTeammate.data.boosts.data.B_SPD = value; return;
	case FV_SPEED:
		data.status.data.cTeammate.data.boosts.data.B_SPE = value; return;
	case FV_ACCURACY:
		data.status.data.cTeammate.data.boosts.data.B_ACC = value; return;
	case FV_EVASION:
		data.status.data.cTeammate.data.boosts.data.B_EVA = value; return;
	case FV_CRITICALHIT:
		data.status.data.cTeammate.data.boosts.data.B_CHT = value; return;
	default:
	case FV_HITPOINTS:
		return;
	}
};





uint32_t team_volatile::cGetFV_boosted(const team_nonvolatile& tNV, size_t type, int32_t tempBoost) const
{
	return cGetFV_boosted(tNV.getPKNV(*this), type, tempBoost);
}

uint32_t team_volatile::cGetFV_boosted(const pokemon_nonvolatile& cPKNV, size_t type, int32_t tempBoost) const
{
	int32_t cBoost = cGetBoost(type) + tempBoost;
	cBoost = std::min(std::max(cBoost, -6), 6);
	return cPKNV.FV_base[type][cBoost + 6];
}

fpType team_volatile::cGetAccuracy_boosted(size_t type, int32_t tempBoost) const
{
	int32_t cBoost = cGetBoost(type) + tempBoost;
	cBoost = std::min(std::max(cBoost, -6), 6);
	return pokemon_nonvolatile::aFV_base[type - 6][cBoost + 6];
}





uint32_t team_volatile::numTeammatesAlive() const
{
	uint32_t result = 0; // accumulate living teammates

	result += teammate(0).isAlive()?1:0;
	result += teammate(1).isAlive()?1:0;
	result += teammate(2).isAlive()?1:0;
	result += teammate(3).isAlive()?1:0;
	result += teammate(4).isAlive()?1:0;
	result += teammate(5).isAlive()?1:0;
	
	return result;
}





bool team_volatile::swapPokemon(size_t iAction, bool preserveVolatile)
{
	size_t iPokemon = iAction - AT_SWITCH_0;

	// make sure we're not switching to ourself
	if (iPokemon == getICPKV()) { return false; } 
	
	// reset the volatile status:
	if (!preserveVolatile) { resetVolatile(); };

	// rewrite swap pokemon value:
	data.status.data.nonvolatile.data.iCPokemon = (uint8_t)iPokemon;
	
	return true;
}





void team_volatile::resetVolatile()
{
	// completely zero bitset
	data.status.data.cTeammate.raw[0] = 0; // zero all boosts
	data.status.data.cTeammate.raw[1] = 0; // zero status word 0
	data.status.data.cTeammate.raw[2] = 0; // zero status word 1
}





bool team_volatile::cModBoost(size_t type, int quantity)
{
	int32_t _buff = cGetBoost(type) + quantity;
	
	if (_buff > 6) _buff = 6;
	else if (_buff < -6) _buff = -6;
	
	// update FV value
	if (_buff != cGetBoost(type))
	{
		// there was an increment / decrement of buff and it was within the range of acceptable values
		cSetBoost(type, _buff);
	
		// true if a change was made
		return true;
	}
	else
	{
		// false if no change made
		return false;
	}
}





bool team_volatile::cHasPP() const
{
	bool result = true;
	const pokemon_volatile& cPKV = getPKV();

	result =  
		(cPKV.getMV(AT_MOVE_0).hasPP()) ||
		(cPKV.getMV(AT_MOVE_1).hasPP()) ||
		(cPKV.getMV(AT_MOVE_2).hasPP()) ||
		(cPKV.getMV(AT_MOVE_3).hasPP());

	return result;
}





bool team_volatile::cIsAlive() const
{
	return getPKV().isAlive();
}





void team_volatile::cModHP(const pokemon_nonvolatile& nv, int32_t quantity)
{
	pokemon_volatile& cPKV = getPKV();
	cPKV.modHP(nv, quantity);
	if (!cPKV.isAlive()) { resetVolatile(); }
}

void team_volatile::cSetHP(const pokemon_nonvolatile& nv, uint32_t amt)
{
	pokemon_volatile& cPKV = getPKV();
	cPKV.setHP(nv, amt);
	if (!cPKV.isAlive()) { resetVolatile(); }
}

void team_volatile::cSetPercentHP(const pokemon_nonvolatile& nv, fpType percent)
{
	pokemon_volatile& cPKV = getPKV();
	cPKV.setPercentHP(nv, percent);
	if (!cPKV.isAlive()) { resetVolatile(); }
}

void team_volatile::cModPercentHP(const pokemon_nonvolatile& nv, fpType percent)
{
	pokemon_volatile& cPKV = getPKV();
	cPKV.modPercentHP(nv, percent);
	if (!cPKV.isAlive()) { resetVolatile(); }
}