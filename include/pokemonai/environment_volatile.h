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
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/join.hpp>

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

  static EnvironmentVolatileData create(
      const EnvironmentNonvolatile& envNV, size_t numActivePokemon = 1);

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

  std::vector<Actor> getActiveActors() const;
  size_t getNumActivePokemon() const;

  /**
   * @brief Get all candidate (move and swap) actions for the given actor.
   * @param actor The actor performing the action
   * @return A vector of candidate actions
   */
  std::vector<Action> getActions(const Actor& actor) const;

  /**
   * @brief Get all candidate actions for the given actor and move
   * @param actor The actor performing the move
   * @param move The move to generate actions for
   * @return A vector of candidate actions
   */
  std::vector<Action> getActions(
      const Actor& actor, const MoveNonVolatile& move) const;

  /**
   * @brief Get all move candidates for the given actor (struggle, wait, and all moves)
   * @param actor The actor to generate candidates for
   * @return A vector of candidate actions
   */
  std::vector<Action> getMoveActions(const Actor& actor) const;

  /**
   * @brief Get all swap candidates for the given actor
   * @param actor The actor to generate candidates for
   * @return A vector of candidate actions
   */
  std::vector<Action> getSwapActions(const Actor& actor) const;

  /**
   * @brief Get all valid targets for the given actor and action
   * @param actor The actor performing the action
   * @param action The action to get targets for
   * @return A vector of valid targets
   */
  std::vector<Actor> getTargets(const Actor& actor, const Action& action) const;

  /**
   * @brief Get the default target for the given actor and action
   * @param actor The actor performing the action
   * @param action The action to get the default target for
   * @return The default target
   */
  Actor defaultTarget(const Actor& actor, const Action& action) const;

 protected:
  Actor defaultEnemy(const Actor& actor, const Action& action) const;
  Actor defaultFriendly(const Actor& actor, const Action& action) const;

 public:
  auto yieldActors(size_t movesFirst = TEAM_A) const {
    return nv().yieldActors(movesFirst);
  }

  auto yieldActiveActors(size_t movesFirst = TEAM_A) const {
    auto teamA = getTeam(movesFirst).yieldActiveActors();
    auto teamB = getOtherTeam(movesFirst).yieldActiveActors();
    return boost::join(teamA, teamB);
  }

  auto yieldActivePokemon(size_t movesFirst = TEAM_A) const {
    auto teamA = getTeam(movesFirst).yieldActivePokemon();
    auto teamB = getOtherTeam(movesFirst).yieldActivePokemon();
    return boost::join(teamA, teamB);
  }

  size_t numActivePokemon() const {
    size_t numActive = 0;
    for (const auto& actor : yieldActiveActors()) { numActive++; }
    return numActive;
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

  operator ConstEnvironmentVolatile() const { return ConstEnvironmentVolatile{nv(), data()}; };
  void initialize(size_t numActivePokemon = 1);
};


PKAISHARED std::ostream& operator <<(std::ostream& os, const ConstEnvironmentVolatile& environment);

#endif	/* ENVIRONMENT_VOLATILE_H */

