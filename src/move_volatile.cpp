
//#define PKAI_EXPORT
#include "../inc/move_volatile.h"

//#define PKAI_STATIC
#include "../inc/pokemon_volatile.h"
#include "../inc/move_nonvolatile.h"
#include "../inc/move.h"
//#undef PKAI_STATIC

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(MoveVolatile) == sizeof(uint8_t));





bool MoveVolatile::operator ==(const MoveVolatile& other) const
{
  if (raw != other.raw) { return false; }
  
  return true;
}





bool MoveVolatile::operator !=(const MoveVolatile& other) const
{
  return !(*this == other);
}





void MoveVolatile::initialize(const MoveNonVolatile& cMove)
{
  // set PP:
  data.PPcurrent = cMove.getPPMax();

  // set zero status:
  data.status_nonvolatile = 0;
}





bool MoveVolatile::modPP(const MoveNonVolatile& cMove, int32_t value)
{
  int32_t _PPcurrent = data.PPcurrent + value;
  
  data.PPcurrent = (uint8_t) std::min((uint32_t)std::max(_PPcurrent, 0), cMove.getPPMax());

  return _PPcurrent == (int32_t)data.PPcurrent; // will be inequal if _PPcurrent is -1 and PPcurrent is still 0
}





bool MoveVolatile::hasPP() const
{
  // is move out of PP?
  return data.PPcurrent > 0;
}