#include "pokemonai/move_volatile.h"

#include <fmt/color.h>
#include <fmt/format.h>

#include <boost/static_assert.hpp>
#include <cstring>
#include <ostream>

#include "pokemonai/move.h"
#include "pokemonai/move_nonvolatile.h"
#include "pokemonai/type.h"

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


MOVE_VOLATILE_IMPL_TEMPLATE
void MOVE_VOLATILE_IMPL::prettyPrint(
    const std::string& prefix, const std::string& suffix) const {
  const Move& base = getBase();
  fmt::print("{}", prefix);
  fmt::print(
      fmt::emphasis::bold | fg(fmt::color::cyan), "\"{}\"", base.getName());
  fmt::print(" [");
  fmt::print(
      fmt::emphasis::bold | fg(fmt::color::yellow),
      "{}",
      base.getType().getName());
  fmt::print("] ");

  fmt::color catColor;
  switch (base.getDamageType()) {
  case 0:
    catColor = fmt::color::white;
    break;
  case 1:
    catColor = fmt::color::crimson;
    break;
  case 2:
    catColor = fmt::color::cornflower_blue;
    break;
  case 3:
    catColor = fmt::color::gray;
    break;
  default:
    catColor = fmt::color::white;
    break;
  }
  fmt::print(
      fmt::emphasis::bold | fg(catColor), "{}", base.getDamageTypeName());

  fmt::print(" Pow: ");
  fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "{}", base.getPower());
  fmt::print(" PP: {}/{}{}\n", (unsigned int)getPP(), nv().getPPMax(), suffix);

  if (!base.getDescription().empty()) {
    fmt::print("\t   \033[3m{}\033[0m\n", base.getDescription());
  }
}


std::ostream& operator <<(std::ostream& os, const ConstMoveVolatile& cMV)
{
  const Move& base = cMV.getBase();

  os << fmt::format(
      "\"{}\" [{}] {} Pwr: {} PP: {}/{}",
      base.getName(),
      base.getType().getName(),
      base.getDamageTypeName(),
      base.getPower(),
      (unsigned int)cMV.getPP(),
      cMV.nv().getPPMax());

  return os;
}


template class MoveVolatileImpl<const MoveVolatileData>;
template class MoveVolatileImpl<MoveVolatileData>;