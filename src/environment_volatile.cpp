#include "pokemonai/environment_volatile.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <boost/static_assert.hpp>
#include <cstring>
#include <ostream>

#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/signature.h"
#include "pokemonai/team_nonvolatile.h"

BOOST_STATIC_ASSERT(sizeof(EnvironmentVolatileData) == (sizeof(uint64_t)*17));


void EnvironmentVolatile::initialize(size_t numActivePokemon) {
  // zero datastructure:
  std::memset(data_, 0, sizeof(EnvironmentVolatileData));
  // initialize:
  getTeam(0).initialize(numActivePokemon);
  getTeam(1).initialize(numActivePokemon);
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


EnvironmentVolatileData EnvironmentVolatileData::create(
    const EnvironmentNonvolatile& envNV, size_t numActivePokemon) {
  EnvironmentVolatileData result;
  EnvironmentVolatile{envNV, result}.initialize(numActivePokemon);
  return result;
};


bool EnvironmentVolatileData::operator ==(const EnvironmentVolatileData& other) const {
  return std::memcmp(this, &other, sizeof(EnvironmentVolatileData)) == 0;
}


bool EnvironmentVolatileData::operator !=(const EnvironmentVolatileData& other) const {
  return !(*this == other);
}


ENV_VOLATILE_IMPL_TEMPLATE
std::vector<Actor> ENV_VOLATILE_IMPL::getActiveActors() const {
  std::vector<Actor> result;
  for (const Actor& actor : yieldActiveActors()) { result.push_back(actor); }
  return result;
}


ENV_VOLATILE_IMPL_TEMPLATE
size_t ENV_VOLATILE_IMPL::getNumActivePokemon() const {
  size_t result = 0;
  for (const Actor& actor : yieldActiveActors()) {
    (void)actor;
    ++result;
  }
  return result;
}


ENV_VOLATILE_IMPL_TEMPLATE
void ENV_VOLATILE_IMPL::printActivePokemon(std::ostream& os, size_t first) const {
  // TODO: print actor id, only print agent/other once
  for (const Actor& actor : yieldActiveActors()) {
    if (actor.iTeam() == first) {
      os << fmt::format(
          "\tagent: {}\n", fmt::streamed(teammate(actor)));
    } else {
      os << fmt::format(
          "\tother: {}\n", fmt::streamed(teammate(actor)));
    }
  }
}


std::ostream& operator <<(std::ostream& os, const ConstEnvironmentVolatile& env) {
  env.printActivePokemon(os);
  return os;
};


template class EnvironmentVolatileImpl<ConstTeamVolatile, const EnvironmentVolatileData>;
template class EnvironmentVolatileImpl<TeamVolatile, EnvironmentVolatileData>;
