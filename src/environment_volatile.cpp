#include "../inc/environment_volatile.h"

#include <boost/static_assert.hpp>
#include <cstring>
#include <ostream>

#include "../inc/team_nonvolatile.h"
#include "../inc/environment_nonvolatile.h"

BOOST_STATIC_ASSERT(sizeof(EnvironmentVolatileData) == (sizeof(uint64_t)*16));


void EnvironmentVolatile::initialize() {
  // zero datastructure:
  std::memset(data_, 0, sizeof(EnvironmentVolatileData));
  // initialize:
  getTeam(0).initialize();
  getTeam(1).initialize();
}


EnvironmentVolatileData EnvironmentVolatileData::create(const EnvironmentNonvolatile& envNV) {
  EnvironmentVolatileData result;
  EnvironmentVolatile{envNV, result}.initialize();
  return result;
};


bool EnvironmentVolatileData::operator ==(const EnvironmentVolatileData& other) const {
  return std::memcmp(this, &other, sizeof(EnvironmentVolatileData)) == 0;
}


bool EnvironmentVolatileData::operator !=(const EnvironmentVolatileData& other) const {
  return !(*this == other);
}


ENV_VOLATILE_IMPL_TEMPLATE
void ENV_VOLATILE_IMPL::printActivePokemon(std::ostream& os, size_t first) const {
  os << "\tagent: " << getTeam(first).getPKV();
  os << "\tother: " << getOtherTeam(first).getPKV();
}


template class EnvironmentVolatileImpl<ConstTeamVolatile, const EnvironmentVolatileData>;
template class EnvironmentVolatileImpl<TeamVolatile, EnvironmentVolatileData>;
