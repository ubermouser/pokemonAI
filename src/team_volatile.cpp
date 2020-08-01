
//#define PKAI_EXPORT
#include "../inc/team_volatile.h"
//#undef PKAI_EXPORT

#include "../inc/pokemon_nonvolatile.h"
#include "../inc/team_nonvolatile.h"

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(TeamVolatile) == (sizeof(uint64_t)*8));
BOOST_STATIC_ASSERT(sizeof(VolatileStatus) == (sizeof(uint32_t)*3));
BOOST_STATIC_ASSERT(sizeof(NonVolatileStatus) == (sizeof(uint32_t)*1));





bool TeamVolatile::operator ==(const TeamVolatile& other) const
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





bool TeamVolatile::operator !=(const TeamVolatile& other) const
{
  return !(*this == other);
}





void TeamVolatile::initialize(const TeamNonVolatile& nv)
{
  // status and all other variables have been zeroed from a different context 
  for (size_t iTeammate = 0, _numTeammates = nv.getNumTeammates(); iTeammate != _numTeammates; ++iTeammate)
  {
    teammate(iTeammate).initialize(nv.teammate(iTeammate));
  }
}





int32_t TeamVolatile::cGetBoost(size_t type) const
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





void TeamVolatile::cSetBoost(size_t type, int32_t value)
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





uint32_t TeamVolatile::cGetFV_boosted(const TeamNonVolatile& tNV, size_t type, int32_t tempBoost) const
{
  return cGetFV_boosted(tNV.getPKNV(*this), type, tempBoost);
}

uint32_t TeamVolatile::cGetFV_boosted(const PokemonNonVolatile& cPKNV, size_t type, int32_t tempBoost) const
{
  int32_t cBoost = cGetBoost(type) + tempBoost;
  cBoost = std::min(std::max(cBoost, -6), 6);
  return cPKNV.FV_base[type][cBoost + 6];
}

fpType TeamVolatile::cGetAccuracy_boosted(size_t type, int32_t tempBoost) const
{
  int32_t cBoost = cGetBoost(type) + tempBoost;
  cBoost = std::min(std::max(cBoost, -6), 6);
  return PokemonNonVolatile::aFV_base[type - 6][cBoost + 6];
}





uint32_t TeamVolatile::numTeammatesAlive() const
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





bool TeamVolatile::swapPokemon(size_t iAction, bool preserveVolatile)
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





void TeamVolatile::resetVolatile()
{
  // completely zero bitset
  data.status.data.cTeammate.raw[0] = 0; // zero all boosts
  data.status.data.cTeammate.raw[1] = 0; // zero status word 0
  data.status.data.cTeammate.raw[2] = 0; // zero status word 1
}





bool TeamVolatile::cModBoost(size_t type, int quantity)
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





bool TeamVolatile::cHasPP() const
{
  bool result = true;
  const PokemonVolatile& cPKV = getPKV();

  result =  
    (cPKV.getMV(AT_MOVE_0).hasPP()) ||
    (cPKV.getMV(AT_MOVE_1).hasPP()) ||
    (cPKV.getMV(AT_MOVE_2).hasPP()) ||
    (cPKV.getMV(AT_MOVE_3).hasPP());

  return result;
}





bool TeamVolatile::cIsAlive() const
{
  return getPKV().isAlive();
}





void TeamVolatile::cModHP(const PokemonNonVolatile& nv, int32_t quantity)
{
  PokemonVolatile& cPKV = getPKV();
  cPKV.modHP(nv, quantity);
  if (!cPKV.isAlive()) { resetVolatile(); }
}

void TeamVolatile::cSetHP(const PokemonNonVolatile& nv, uint32_t amt)
{
  PokemonVolatile& cPKV = getPKV();
  cPKV.setHP(nv, amt);
  if (!cPKV.isAlive()) { resetVolatile(); }
}

void TeamVolatile::cSetPercentHP(const PokemonNonVolatile& nv, fpType percent)
{
  PokemonVolatile& cPKV = getPKV();
  cPKV.setPercentHP(nv, percent);
  if (!cPKV.isAlive()) { resetVolatile(); }
}

void TeamVolatile::cModPercentHP(const PokemonNonVolatile& nv, fpType percent)
{
  PokemonVolatile& cPKV = getPKV();
  cPKV.modPercentHP(nv, percent);
  if (!cPKV.isAlive()) { resetVolatile(); }
}