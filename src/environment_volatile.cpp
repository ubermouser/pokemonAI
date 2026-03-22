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
    for (size_t iTeammate = 0; iTeammate < team.nv().getNumTeammates();
         ++iTeammate) {
      if (team.teammate(iTeammate).data().active) {
        result.push_back({iTeam, iTeammate});
      }
    }
  }
  return result;
}


ENV_VOLATILE_IMPL_TEMPLATE
void ENV_VOLATILE_IMPL::printActivePokemon(std::ostream& os, size_t first) const {
  std::string agent_str;
  auto team1 = getTeam(first);
  for (size_t iTeammate = 0; iTeammate < team1.nv().getNumTeammates(); ++iTeammate) {
    if (team1.teammate(iTeammate).data().active) {
      if (!agent_str.empty()) agent_str += ", ";
      agent_str += fmt::format("{}", fmt::streamed(team1.teammate(iTeammate)));
    }
  }

  std::string other_str;
  auto team2 = getOtherTeam(first);
  for (size_t iTeammate = 0; iTeammate < team2.nv().getNumTeammates(); ++iTeammate) {
    if (team2.teammate(iTeammate).data().active) {
      if (!other_str.empty()) other_str += ", ";
      other_str += fmt::format("{}", fmt::streamed(team2.teammate(iTeammate)));
    }
  }

  os << fmt::format(
      "\tagent: {}\n\tother: {}\n",
      agent_str,
      other_str);
}


std::ostream& operator <<(std::ostream& os, const ConstEnvironmentVolatile& env) {
  env.printActivePokemon(os);
  return os;
};


template class EnvironmentVolatileImpl<ConstTeamVolatile, const EnvironmentVolatileData>;
template class EnvironmentVolatileImpl<TeamVolatile, EnvironmentVolatileData>;
