
//#define PKAI_EXPORT
#include "../inc/environment_nonvolatile.h"
//#undef PKAI_EXPORT

#include <assert.h>

//#define PKAI_STATIC
#include "../inc/team_nonvolatile.h"
//#undef PKAI_STATIC

EnvironmentNonvolatile& EnvironmentNonvolatile::initialize() {
  for (size_t iTeam = 0; iTeam < teams.size(); ++iTeam)
  {
    getTeam(iTeam).initialize();
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
  if (init) { getTeam(iTeam).initialize(); };

  return *this;
};
