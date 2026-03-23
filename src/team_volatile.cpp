#include "pokemonai/team_volatile.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <boost/static_assert.hpp>
#include <cstring>
#include <ostream>

#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/team_nonvolatile.h"

BOOST_STATIC_ASSERT(sizeof(NonVolatileStatus) == (sizeof(uint32_t)*1));
BOOST_STATIC_ASSERT(sizeof(VolatileStatus) == (sizeof(uint32_t)*4));
BOOST_STATIC_ASSERT(sizeof(TeamStatus) == (sizeof(uint32_t)*5));
BOOST_STATIC_ASSERT(sizeof(TeamVolatileData) == (sizeof(uint32_t)*17));

bool TeamVolatileData::operator==(const TeamVolatileData& other) const {
  return std::memcmp(this, &other, sizeof(TeamVolatileData)) == 0;
}

bool TeamVolatileData::operator !=(const TeamVolatileData& other) const
{
  return !(*this == other);
}


void TeamVolatile::initialize()
{
  // status and all other variables have been zeroed from a different context
  for (size_t iTeammate = 0, _numTeammates = nv_->getNumTeammates(); iTeammate != _numTeammates; ++iTeammate)
  {
    // 0th teammate is active at the start of the game
    teammate(iTeammate).initialize(iTeammate == 0);
  }
}


TEAM_VOLATILE_IMPL_TEMPLATE
typename TEAM_VOLATILE_IMPL::pokemonvolatile_t
TEAM_VOLATILE_IMPL::teammate(size_t iTeammate) const {
  return pokemonvolatile_t{
      nv().teammate(iTeammate), // contains assertion
      data().teammates[iTeammate],
      data().status
  };
};


TEAM_VOLATILE_IMPL_TEMPLATE
std::vector<size_t> TEAM_VOLATILE_IMPL::getActivePokemon() const {
  std::vector<size_t> result;
  for (size_t iTeammate = 0; iTeammate != nv().getNumTeammates(); ++iTeammate) {
    if (teammate(iTeammate).data().active) {
      result.push_back(iTeammate);
    }
  }
  return result;
}


TEAM_VOLATILE_IMPL_TEMPLATE
size_t TEAM_VOLATILE_IMPL::getNumActivePokemon() const {
  size_t result = 0;
  for (size_t iTeammate = 0; iTeammate != nv().getNumTeammates(); ++iTeammate) {
    if (teammate(iTeammate).data().active) {
      ++result;
    }
  }
  return result;
}


TEAM_VOLATILE_IMPL_TEMPLATE
uint32_t TEAM_VOLATILE_IMPL::numTeammatesAlive() const {
  uint32_t result = 0; // accumulate living teammates
  for (size_t iPokemon= 0; iPokemon != nv().getNumTeammates(); ++iPokemon) {
    result += teammate(iPokemon).isAlive();
  }

  return result;
}


TEAM_VOLATILE_IMPL_TEMPLATE
bool TEAM_VOLATILE_IMPL::isAlive() const {
  size_t numTeammates = nv().getNumTeammates();
  size_t iCurrent = getICPKV();
  for (size_t iPokemon=iCurrent, iMax=iCurrent+numTeammates; iPokemon < iMax; ++iPokemon) {
    if (teammate(iPokemon % numTeammates).isAlive()) { return true; }
  }

  return false;
}


bool TeamVolatile::swapPokemon(size_t iAction, bool preserveVolatile)
{
  size_t iOldPokemon = getICPKV();
  size_t iNewPokemon = iAction;

  // make sure we're not switching to ourself
  if (iNewPokemon == iOldPokemon) { return false; }

  // reset the volatile status:
  if (!preserveVolatile) { resetVolatile(); };

  // rewrite swap pokemon value:
  data_->status.nonvolatile.iCPokemon = (uint8_t)iNewPokemon;
  teammate(iOldPokemon).data().active = false;
  teammate(iNewPokemon).data().active = true;

  return true;
}


TEAM_VOLATILE_IMPL_TEMPLATE
void TEAM_VOLATILE_IMPL::printTeam(std::ostream& os, const std::string& linePrefix) const {
  for (size_t iTeammate = 0; iTeammate != nv().getNumTeammates(); ++iTeammate) {
    fmt::print(
        os,
        "{}{}-{}\n",
        linePrefix,
        iTeammate,
        fmt::streamed(teammate(iTeammate)));
  }
}


void TeamVolatile::resetVolatile()
{
  // completely zero bitset
  std::memset(&(data().status.cTeammate), 0, sizeof(VolatileStatus));
}


std::ostream& operator <<(std::ostream& os, const ConstTeamVolatile& team) {
  team.printTeam(os);
  return os;
}


template class TeamVolatileImpl<ConstPokemonVolatile, const TeamVolatileData, const TeamStatus>;
template class TeamVolatileImpl<PokemonVolatile, TeamVolatileData, TeamStatus>;