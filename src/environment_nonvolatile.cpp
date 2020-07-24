
//#define PKAI_EXPORT
#include "../inc/environment_nonvolatile.h"
//#undef PKAI_EXPORT

#include <assert.h>

//#define PKAI_STATIC
#include "../inc/team_nonvolatile.h"
//#undef PKAI_STATIC

void environment_nonvolatile::initialize()
{
  for (size_t iTeam = 0; iTeam < teams.size(); ++iTeam)
  {
    getTeam(iTeam).initialize();
  }
};





void environment_nonvolatile::uninitialize()
{
    for (size_t iTeam = 0; iTeam < teams.size(); ++iTeam)
  {
    getTeam(iTeam).uninitialize();
  }
};





void environment_nonvolatile::setTeam(size_t iTeam, const team_nonvolatile& cTeam, bool init)
{
  assert(iTeam < 2);
  teams[iTeam] = cTeam;
  if (init) { getTeam(iTeam).initialize(); };
};
