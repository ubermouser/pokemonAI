/* 
 * File:   PKAI_environment_volatile.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:08 PM
 */

#ifndef ENVIRONMENT_VOLATILE_H
#define	ENVIRONMENT_VOLATILE_H

#include "pkai.h"

#include <array>
#include <ostream>
#include <stdint.h>

#include "environment_nonvolatile.h"
#include "nonvolatile_volatile.h"
#include "team_volatile.h"

class EnvironmentNonvolatile;

struct PKAISHARED EnvironmentVolatileData
{
  std::array<TeamVolatileData, 2> teams;
  
  /* Compares values of selected environment. Base values are compared by
   * pointer, volatile values are compared by value */
  bool operator==(const EnvironmentVolatileData& other) const;
  bool operator!=(const EnvironmentVolatileData& other) const;

  static EnvironmentVolatileData create(const EnvironmentNonvolatile& envNV);

  uint64_t generateHash() const;
};


#define ENV_VOLATILE_IMPL_TEMPLATE template<typename TeamVolatileType, typename VolatileType>
#define ENV_VOLATILE_IMPL EnvironmentVolatileImpl<TeamVolatileType, VolatileType>

ENV_VOLATILE_IMPL_TEMPLATE
class PKAISHARED EnvironmentVolatileImpl: public NonvolatileVolatilePair<const EnvironmentNonvolatile, VolatileType> {
public:
  using base_t = NonvolatileVolatilePair<const EnvironmentNonvolatile, VolatileType>;
  using impl_t = ENV_VOLATILE_IMPL;
  using teamvolatile_t = TeamVolatileType;
  using base_t::base_t;
  using base_t::data;
  using base_t::nv;

  teamvolatile_t getTeam(size_t movesFirst) const {
    return teamvolatile_t{nv().getTeam(movesFirst), data().teams[movesFirst]};
  };
  
  teamvolatile_t getOtherTeam(size_t movesFirst) const {
    return getTeam((movesFirst + 1) & 1);
  }

  void printActivePokemon(std::ostream& os, size_t firstTeam=0) const;
};


class PKAISHARED ConstEnvironmentVolatile: public EnvironmentVolatileImpl<ConstTeamVolatile, const EnvironmentVolatileData> {
public:
  using impl_t::impl_t;
};


class PKAISHARED EnvironmentVolatile: public EnvironmentVolatileImpl<TeamVolatile, EnvironmentVolatileData> {
public:
  using impl_t::impl_t;

  operator ConstEnvironmentVolatile() { return ConstEnvironmentVolatile{nv(), data()}; };
  void initialize();
};


PKAISHARED std::ostream& operator <<(std::ostream& os, const ConstEnvironmentVolatile& environment);

#endif	/* ENVIRONMENT_VOLATILE_H */

