/* 
 * File:   PKAI_environment_volatile.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:08 PM
 */

#ifndef ENVIRONMENT_VOLATILE_H
#define	ENVIRONMENT_VOLATILE_H

#include <stdint.h>

#include <array>
#include <iosfwd>
#include <vector>

#include "actor.h"
#include "environment_nonvolatile.h"
#include "nonvolatile_volatile.h"
#include "pkai.h"
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

  typename teamvolatile_t::pokemonvolatile_t teammate(
      const Actor& actor) const {
    return getTeam(actor.iTeam()).teammate(actor.iTeammate());
  }

  typename teamvolatile_t::pokemonvolatile_t teammate(
      size_t iTeam, size_t iTeammate) const {
    return teammate({iTeam, iTeammate});
  }

  std::vector<Actor> getActivePokemon() const;
  size_t getNumActivePokemon() const;

  struct ActivePokemonRange {
    const EnvironmentNonvolatile* nv;
    const EnvironmentVolatileData* data;
    struct Iterator {
      const EnvironmentNonvolatile* nv;
      const EnvironmentVolatileData* data;
      size_t iTeam;
      typename teamvolatile_t::ActivePokemonRange::Iterator teamIt;

      void advance() {
        while (iTeam < 2) {
          if (teamIt.index < nv->getTeam(iTeam).getNumTeammates()) {
            return;
          }
          ++iTeam;
          if (iTeam < 2) {
            teamIt = {&nv->getTeam(iTeam), &data->teams[iTeam], 0};
            teamIt.advance();
          }
        }
      }

      Actor operator*() const { return {iTeam, *teamIt}; }
      Iterator& operator++() {
        ++teamIt;
        advance();
        return *this;
      }
      bool operator==(const Iterator& other) const {
        if (iTeam != other.iTeam) return false;
        if (iTeam >= 2) return true;
        return teamIt == other.teamIt;
      }
      bool operator!=(const Iterator& other) const { return !(*this == other); }

      using iterator_category = std::input_iterator_tag;
      using value_type = Actor;
      using difference_type = std::ptrdiff_t;
      using pointer = const Actor*;
      using reference = const Actor&;
    };

    Iterator begin() const {
      Iterator it{nv, data, 0, {&nv->getTeam(0), &data->teams[0], 0}};
      it.teamIt.advance();
      it.advance();
      return it;
    }
    Iterator end() const {
      return Iterator{
          nv,
          data,
          2,
          {&nv->getTeam(1),
           &data->teams[1],
           nv->getTeam(1).getNumTeammates()}};
    }
  };

  ActivePokemonRange yieldActivePokemon() const { return {&nv(), &data()}; }

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

