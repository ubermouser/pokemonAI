
//#define PKAI_EXPORT
#include "../inc/environment_volatile.h"
//#undef PKAI_EXPORT

#include <string.h>

//#define PKAI_STATIC
#include "../inc/team_nonvolatile.h"
#include "../inc/environment_nonvolatile.h"
#include "../inc/environment_possible.h"
//#undef PKAI_STATIC

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(EnvironmentVolatile) == (sizeof(uint64_t)*16));


EnvironmentVolatile::EnvironmentVolatile(
    const EnvironmentPossible& other) : EnvironmentVolatile(other.getEnv()) {
}


void EnvironmentVolatile::initialize(const EnvironmentNonvolatile& envNV)
{
  // zero datastructure:
  memset(raw, 0, sizeof(EnvironmentVolatile));
  // initialize:
  data.teams[0].initialize(envNV.getTeam(0));
  data.teams[1].initialize(envNV.getTeam(1));
}





EnvironmentVolatile EnvironmentVolatile::create(const EnvironmentNonvolatile& envNV)
{
  EnvironmentVolatile result;
  result.initialize(envNV);
  return result;
};





bool EnvironmentVolatile::operator ==(const EnvironmentVolatile& other) const
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





bool EnvironmentVolatile::operator !=(const EnvironmentVolatile& other) const
{
  return !(*this == other);
}





const TeamVolatile& EnvironmentVolatile::getTeam(size_t movesFirst) const
{
  return data.teams[movesFirst];
}





const TeamVolatile& EnvironmentVolatile::getOtherTeam(size_t movesFirst) const
{
  return data.teams[(movesFirst + 1) & 1];
}





TeamVolatile& EnvironmentVolatile::getTeam(size_t movesFirst)
{
  return data.teams[movesFirst];
}





TeamVolatile& EnvironmentVolatile::getOtherTeam(size_t movesFirst)
{
  return data.teams[(movesFirst + 1) & 1];
}




