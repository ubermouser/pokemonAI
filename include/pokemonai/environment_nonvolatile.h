#ifndef ENVIRONMENT_NONVOLATILE_H
#define	ENVIRONMENT_NONVOLATILE_H

#include <stdint.h>

#include <array>
#include <boost/range/join.hpp>

#include "pkai.h"
#include "team_nonvolatile.h"

class PKAISHARED EnvironmentNonvolatile {
public:
  std::array<TeamNonVolatile, 2> teams;

  EnvironmentNonvolatile()
    : teams()
  {
  };

  EnvironmentNonvolatile(const TeamNonVolatile& _teamA, const TeamNonVolatile& _teamB, bool init = false)
  {
    teams[0] = _teamA;
    teams[1] = _teamB;
    if (init) { initialize(); };
  };

  EnvironmentNonvolatile(const EnvironmentNonvolatile& other)
    : teams(other.teams)
  {
  };

  TeamNonVolatile& getTeam(size_t movesFirst) {
    return teams[movesFirst];
  };
  const TeamNonVolatile& getTeam(size_t movesFirst) const {
    return teams[movesFirst];
  };

  TeamNonVolatile& getOtherTeam(size_t movesFirst) {
    return teams[(movesFirst + 1) & 1];
  };
  const TeamNonVolatile& getOtherTeam(size_t movesFirst) const {
    return teams[(movesFirst + 1) & 1];
  };

  const PokemonNonVolatile& teammate(
      size_t movesFirst, size_t iTeammate) const {
    return getTeam(movesFirst).teammate(iTeammate);
  }
  PokemonNonVolatile& teammate(size_t movesFirst, size_t iTeammate) {
    return getTeam(movesFirst).teammate(iTeammate);
  }

  EnvironmentNonvolatile& initialize();

  void uninitialize();

  size_t getNumPokemon() const {
    return getTeam(0).getNumTeammates() + getTeam(1).getNumTeammates();
  }

  auto yieldActors(size_t movesFirst = TEAM_A) const {
    auto teamA = getTeam(movesFirst).yieldActors();
    auto teamB = getOtherTeam(movesFirst).yieldActors();
    return boost::join(teamA, teamB);
  };

  auto yieldPokemon(size_t movesFirst = TEAM_A) const {
    auto teamA = getTeam(movesFirst).yieldPokemon();
    auto teamB = getOtherTeam(movesFirst).yieldPokemon();
    return boost::join(teamA, teamB);
  };

  EnvironmentNonvolatile& setTeam(size_t iTeam, const TeamNonVolatile& cTeam, bool init = false);

};

#endif /* ENVIRONMENT_NONVOLATILE_H */
