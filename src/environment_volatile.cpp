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
std::vector<Actor> ENV_VOLATILE_IMPL::getActivePokemon() const {
  std::vector<Actor> result;
  for (size_t iTeam = 0; iTeam < data().teams.size(); ++iTeam) {
    auto team = getTeam(iTeam);
    for (size_t iTeammate : team.getActivePokemon()) {
      result.push_back({iTeam, iTeammate});
    }
  }
  return result;
}


ENV_VOLATILE_IMPL_TEMPLATE
size_t ENV_VOLATILE_IMPL::getNumActivePokemon() const {
  size_t result = 0;
  for (size_t iTeam = 0; iTeam < data().teams.size(); ++iTeam) {
    result += getTeam(iTeam).getNumActivePokemon();
  }
  return result;
}


ENV_VOLATILE_IMPL_TEMPLATE
void ENV_VOLATILE_IMPL::printActivePokemon(std::ostream& os, size_t first) const {
  auto agentTeam = getTeam(first);
  auto otherTeam = getOtherTeam(first);

  for (size_t iTeammate : agentTeam.getActivePokemon()) {
    os << fmt::format(
        "\tagent: {}\n", fmt::streamed(agentTeam.teammate(iTeammate)));
  }
  for (size_t iTeammate : otherTeam.getActivePokemon()) {
    os << fmt::format(
        "\tother: {}\n", fmt::streamed(otherTeam.teammate(iTeammate)));
  }
}


std::ostream& operator <<(std::ostream& os, const ConstEnvironmentVolatile& env) {
  env.printActivePokemon(os);
  return os;
};


template class EnvironmentVolatileImpl<ConstTeamVolatile, const EnvironmentVolatileData>;
template class EnvironmentVolatileImpl<TeamVolatile, EnvironmentVolatileData>;
