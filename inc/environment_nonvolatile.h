#ifndef ENVIRONMENT_NONVOLATILE_H
#define	ENVIRONMENT_NONVOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <array>

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

  TeamNonVolatile& getTeam(size_t movesFirst)
  {
    return teams[movesFirst];
  };

  TeamNonVolatile& getOtherTeam(size_t movesFirst)
  {

    return teams[(movesFirst+1)&1];
  };

  void initialize();

  void uninitialize();

  const TeamNonVolatile& getTeam(size_t movesFirst) const
  {
    return teams[movesFirst];
  };

  const TeamNonVolatile& getOtherTeam(size_t movesFirst) const
  {
    return teams[(movesFirst+1)&1];
  };

  void setTeam(size_t iTeam, const TeamNonVolatile& cTeam, bool init = false);

};

#endif /* ENVIRONMENT_NONVOLATILE_H */
