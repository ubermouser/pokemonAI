/* 
 * File:   PKAI_team_volatile.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:04 PM
 */

#ifndef TEAM_VOLATILE_H
#define	TEAM_VOLATILE_H

#include <stdint.h>

#include <array>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include <iosfwd>
#include <vector>

#include "nonvolatile_volatile.h"
#include "pkai.h"
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

  pokemonvolatile_t teammate(const Actor& actor) const {
    return teammate(actor.iTeammate());
  }
  pokemonvolatile_t teammate(const Action& action) const { return teammate(action.iFriendly()); }
  pokemonvolatile_t teammate(size_t iTeammate) const;

  /* returns number of teammates on this team that are still alive */
  uint32_t numTeammatesAlive() const;

  /* returns indices of teammates on this team that are active */
  std::vector<Actor> getActiveActors() const;

  /* returns number of teammates on this team that are active */
  size_t getNumActivePokemon() const;

  auto yieldActors(size_t iCurrent = 0) const {
    return nv().yieldActors(iCurrent);
  }

  auto yieldPokemon(size_t iCurrent = 0) const {
    return yieldActors(iCurrent) |
           boost::adaptors::transformed([*this](const auto& actor) {
             return std::make_tuple(actor, teammate(actor));
           });
  }

  auto yieldActiveActors(size_t iCurrent = 0) const {
    return yieldActors(iCurrent) |
           boost::adaptors::filtered([*this](const auto& actor) {
             return teammate(actor).isActive();
           });
  }


  auto yieldInactiveActors(size_t iCurrent = 0) const {
    return yieldActors(iCurrent) |
           boost::adaptors::filtered([*this](const auto& actor) {
             return !teammate(actor).isActive();
           });
  }

  auto yieldActivePokemon(size_t iCurrent = 0) const {
    return yieldActiveActors(iCurrent) |
           boost::adaptors::transformed([*this](const auto& actor) {
             return std::make_tuple(actor, teammate(actor));
           });
  }

  auto yieldInactivePokemon(size_t iCurrent = 0) const {
    return yieldInactiveActors(iCurrent) |
           boost::adaptors::transformed([*this](const auto& actor) {
             return std::make_tuple(actor, teammate(actor));
           });
  }

  /* returns true if at least one teammate on the team is alive */
  bool isAlive() const;

  /* Retrieves a pointer to the current pokemon active on this team */
  [[deprecated]] pokemonvolatile_t getPKV() const {
    return teammate(getICPKV());
  }

  /* gets current index of pokemon volatile on this team */
  [[deprecated]] size_t getICPKV() const {
    return data().status.nonvolatile.iCPokemon;
  };

  status_t& status() const { return data().status; }

  void printTeam(std::ostream& os, const std::string& linePrefix="") const;
};


class PKAISHARED ConstTeamVolatile: public TeamVolatileImpl<ConstPokemonVolatile, const TeamVolatileData, const TeamStatus> {
public:
  using impl_t::impl_t;
};


class PKAISHARED TeamVolatile: public TeamVolatileImpl<PokemonVolatile, TeamVolatileData, TeamStatus> {
public:
  using impl_t::impl_t;

  operator ConstTeamVolatile() const { return ConstTeamVolatile{nv(), data()}; };

  void resetVolatile();

  
  VolatileStatus& getVolatile() { return data().status.cTeammate; };
  const VolatileStatus& getVolatile() const { return data().status.cTeammate; };

  NonVolatileStatus& getNonVolatile() { return data().status.nonvolatile; };
  const NonVolatileStatus& getNonVolatile() const { return data().status.nonvolatile; };

  /* Resets all pokemon in this team */
  void initialize(size_t numActivePokemon);

  /* Swaps the currently active pokemon with the target pokemon */
  [[deprecated]] bool swapPokemon(
      size_t iAction, bool preserveVolatile = false);

  bool swapPokemon(
      const Actor& actor, const Actor& target, bool preserveVolatile = false);

  /* Activates a currently sidelined pokemon, returning their position */
  size_t activatePokemon(const Actor& actor);
};

PKAISHARED std::ostream& operator <<(std::ostream& os, const ConstTeamVolatile& team);

#endif	/* TEAM_VOLATILE_H */

