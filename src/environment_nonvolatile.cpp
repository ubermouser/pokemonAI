
#include "pokemonai/environment_nonvolatile.h"

#include <assert.h>

#include "pokemonai/team_nonvolatile.h"

EnvironmentNonvolatile& EnvironmentNonvolatile::initialize() {
  for (size_t iTeam = 0; iTeam < teams.size(); ++iTeam)
  {
    auto& team = getTeam(iTeam);
    team.team_ = (TEAM)iTeam;
    team.initialize();
  }
  return *this;
};


void EnvironmentNonvolatile::uninitialize() {
    for (size_t iTeam = 0; iTeam < teams.size(); ++iTeam)
  {
    getTeam(iTeam).uninitialize();
  }
};


EnvironmentNonvolatile& EnvironmentNonvolatile::setTeam(size_t iTeam, const TeamNonVolatile& cTeam, bool init) {
  assert(iTeam < 2);
  teams[iTeam] = cTeam;
  teams[iTeam].team_ = (TEAM)iTeam;
  if (init) { getTeam(iTeam).initialize(); };

  return *this;
};
