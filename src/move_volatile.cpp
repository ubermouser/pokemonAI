#include "../inc/move_volatile.h"

#include <boost/static_assert.hpp>
#include <cstring>

#include "../inc/move_nonvolatile.h"
#include "../inc/move.h"

BOOST_STATIC_ASSERT(sizeof(MoveVolatileData) == sizeof(uint8_t));


bool MoveVolatileData::operator ==(const MoveVolatileData& other) const
{
  return std::memcmp(this, &other, sizeof(MoveVolatileData)) == 0;
}


bool MoveVolatileData::operator !=(const MoveVolatileData& other) const
{
  return !(*this == other);
}


void MoveVolatile::initialize()
{
  // set PP:
  data().PPcurrent = nv().getPPMax();

  // set zero status:
  data().status_nonvolatile = 0;
}


bool MoveVolatile::modPP(int32_t _value) {
  int32_t value = data().PPcurrent + _value;
  value = std::max(value, 0);

  return setPP(value);
}


bool MoveVolatile::setPP(uint32_t value) {
  data().PPcurrent = (uint8_t)std::min(value, nv().getPPMax());

  return value == (int32_t)data().PPcurrent; // will be inequal if _PPcurrent is -1 and PPcurrent is still 0
}


std::ostream& operator <<(std::ostream& os, const ConstMoveVolatile& cMV)
{
  os << "\"" << cMV.getBase().getName() << "\" " << (unsigned int)cMV.getPP() << "/" << cMV.nv().getPPMax();

  return os;
}
