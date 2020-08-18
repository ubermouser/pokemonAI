/* 
 * File:   PKAI_team_volatile.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:04 PM
 */

#ifndef TEAM_VOLATILE_H
#define	TEAM_VOLATILE_H

#include "pkai.h"

#include <array>
#include <ostream>
#include <stdint.h>

#include "nonvolatile_volatile.h"
#include "pokemon_volatile.h"
#include "team_nonvolatile.h"
#include "team_status.h"

class TeamNonVolatile;
class PokemonNonVolatile;


struct PKAISHARED TeamVolatileData
{
  /* storage for 6 pokemon. Unused pokemon are zeroed */
  std::array<PokemonVolatileData, 6> teammates;

  TeamStatus status;


  /* Compares values of selected team. Base values are compared by
    * pointer, volatile values are compared by value */
  bool operator==(const TeamVolatileData& other) const;
  bool operator!=(const TeamVolatileData& other) const;
};


#define TEAM_VOLATILE_IMPL_TEMPLATE template<typename PokemonVolatileType, typename VolatileType, typename StatusType>
#define TEAM_VOLATILE_IMPL TeamVolatileImpl<PokemonVolatileType, VolatileType, StatusType>

TEAM_VOLATILE_IMPL_TEMPLATE
class PKAISHARED TeamVolatileImpl: public NonvolatileVolatilePair<const TeamNonVolatile, VolatileType> {
public:
  using base_t = NonvolatileVolatilePair<const TeamNonVolatile, VolatileType>;
  using impl_t = TEAM_VOLATILE_IMPL;
  using pokemonvolatile_t = PokemonVolatileType;
  using status_t = StatusType;
  using base_t::base_t;
  using base_t::data;
  using base_t::nv;

  const VolatileStatus& getVolatile() const { return data().status.cTeammate; };
  const NonVolatileStatus& getNonVolatile() const { return data().status.nonvolatile; };

  pokemonvolatile_t teammate(size_t iTeammate) const;

  /* returns number of teammates on this team that are still alive */
  uint32_t numTeammatesAlive() const;

  /* Retrieves a pointer to the current pokemon active on this team */
  pokemonvolatile_t getPKV() const { return teammate(getICPKV()); }

  /* gets current index of pokemon volatile on this team */
  size_t getICPKV() const { return data().status.nonvolatile.iCPokemon; };

  status_t& status() const { return data().status; }

  int32_t cGetBoost(size_t type) const { return getPKV().getBoost(type); };

  uint32_t cGetFV_boosted(size_t type, int32_t tempBoost = 0) const {
    return getPKV().getFV_boosted(type, tempBoost);
  };
  fpType cGetAccuracy_boosted(size_t type, int32_t tempBoost = 0) const {
    return getPKV().getAccuracy_boosted(type, tempBoost);
  };

  bool cHasPP() const { return getPKV().hasPP(); };

  /* returns TRUE if the pokemon has more than 0 hitpoints */
  bool cIsAlive() const { return getPKV().isAlive(); }

  void printTeam(std::ostream& os) const;
};


class PKAISHARED ConstTeamVolatile: public TeamVolatileImpl<ConstPokemonVolatile, const TeamVolatileData, const TeamStatus> {
public:
  using impl_t::impl_t;
};


class PKAISHARED TeamVolatile: public TeamVolatileImpl<PokemonVolatile, TeamVolatileData, TeamStatus> {
public:
  using impl_t::impl_t;

  operator ConstTeamVolatile() { return ConstTeamVolatile{nv(), data()}; };

  void resetVolatile();

  
  VolatileStatus& getVolatile() { return data().status.cTeammate; };
  NonVolatileStatus& getNonVolatile() { return data().status.nonvolatile; };

  /* Resets all pokemon in this team */
  void initialize();
  
  /* Swaps the currently active pokemon with the target pokemon */
  bool swapPokemon(size_t iAction, bool preserveVolatile = false);

  void cSetBoost(size_t type, int32_t value) { getPKV().setBoost(type, value); }

  bool cModBoost(size_t type, int32_t amt) { return getPKV().modBoost(type, amt); }

  /* increment target's hp by quantity. */
  void cModHP(int32_t quantity) { getPKV().modHP(quantity); };

  /* set target's hp to quantity. */
  void cSetHP(uint32_t amt) { getPKV().setHP(amt); };

  /* set target's hp to % quantity of total */
  void cSetPercentHP(fpType percent) { getPKV().setPercentHP(percent); };
  
  /* increment target's HP by percent of total. */
  void cModPercentHP(fpType percent) { getPKV().modPercentHP(percent); };
};

#endif	/* TEAM_VOLATILE_H */

