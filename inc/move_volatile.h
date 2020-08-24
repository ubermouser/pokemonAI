
#ifndef MOVE_VOLATILE_H
#define MOVE_VOLATILE_H

#include "pkai.h"

#include <ostream>
#include <stdint.h>

#include "nonvolatile_volatile.h"
#include "move_nonvolatile.h"

class Move;

struct PKAISHARED MoveVolatileData
{
  uint8_t status_nonvolatile : 1;
  uint8_t PPcurrent : 7;

  /* Compares values of selected move. Base values are compared by
   * pointer, volatile values are compared by value
   * DEPRECIATED: hash and compare environment_volatile instead! */
  bool operator==(const MoveVolatileData& other) const;
  bool operator!=(const MoveVolatileData& other) const;
};


template<typename VolatileType>
class MoveVolatileImpl: public NonvolatileVolatilePair<const MoveNonVolatile, VolatileType> {
public:
  using base_t = NonvolatileVolatilePair<const MoveNonVolatile, VolatileType>;
  using base_t::base_t;
  using base_t::data;
  using base_t::nv;

  /* returns percentage of move's PP remaining. */
  fpType getPercentPP() const { return fpType(data().PPcurrent) / fpType(nv().getPPMax()); };

  /* returns count of this move's PP */
  uint32_t getPP() const { return data().PPcurrent; };

  /* can a pokemon use this move during its next turn? */
  bool hasPP() const { return data().PPcurrent > 0; }

  /* retrieve the base move of the current volatile move */
  const Move& getBase() const { return nv().getBase(); };
};


class ConstMoveVolatile: public MoveVolatileImpl<const MoveVolatileData> {
public:
  using base_t = MoveVolatileImpl<const MoveVolatileData>;
  using base_t::base_t;

};


class MoveVolatile: public MoveVolatileImpl<MoveVolatileData> {
public:
  using base_t = MoveVolatileImpl<MoveVolatileData>;
  using base_t::base_t;

  operator ConstMoveVolatile() const { return ConstMoveVolatile{nv(), data()}; };

  /* resets values of PPcurrent and PPmax */
  void initialize();

  /* modifies this move's PP by value */
  bool modPP(int32_t value);

};


PKAISHARED std::ostream& operator <<(std::ostream& os, const ConstMoveVolatile& cM);

#endif	/* MOVE_VOLATILE_H */
