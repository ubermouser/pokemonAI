#include "../inc/environment_volatile.h"

#include <boost/static_assert.hpp>
#include <cstring>
#include <ostream>

#include "../inc/environment_nonvolatile.h"
#include "../inc/signature.h"
#include "../inc/team_nonvolatile.h"

BOOST_STATIC_ASSERT(sizeof(EnvironmentVolatileData) == (sizeof(uint64_t)*16));


void EnvironmentVolatile::initialize() {
  // zero datastructure:
  std::memset(data_, 0, sizeof(EnvironmentVolatileData));
  // initialize:
  getTeam(0).initialize();
  getTeam(1).initialize();
}


uint64_t EnvironmentVolatileData::generateHash() const {
#if defined(_USEFNVHASH)
    return hashes::hash_fnv(this, sizeof(EnvironmentVolatileData));
#elif defined(_USEMURMUR2)
    return hashes::hash_murmur2(this, sizeof(EnvironmentVolatileData));
#else
    return hashes::hash_murmur3(this, sizeof(EnvironmentVolatileData));
#endif
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


std::ostream& operator <<(std::ostream& os, const ConstEnvironmentVolatile& env) {
  env.printActivePokemon(os);
  return os;
};


template class EnvironmentVolatileImpl<ConstTeamVolatile, const EnvironmentVolatileData>;
template class EnvironmentVolatileImpl<TeamVolatile, EnvironmentVolatileData>;
