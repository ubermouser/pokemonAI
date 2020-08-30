#include "../inc/team_volatile.h"

#include "../inc/pokemon_nonvolatile.h"
#include "../inc/team_nonvolatile.h"

#include <cstring>
#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(NonVolatileStatus) == (sizeof(uint32_t)*1));
BOOST_STATIC_ASSERT(sizeof(VolatileStatus) == (sizeof(uint32_t)*3));
BOOST_STATIC_ASSERT(sizeof(TeamStatus) == (sizeof(uint32_t)*4));
BOOST_STATIC_ASSERT(sizeof(TeamVolatileData) == (sizeof(uint64_t)*8));



bool TeamVolatileData::operator ==(const TeamVolatileData& other) const
{	
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
uint32_t TEAM_VOLATILE_IMPL::numTeammatesAlive() const {
  uint32_t result = 0; // accumulate living teammates
  for (size_t iPokemon= 0; iPokemon != nv().getNumTeammates(); ++iPokemon) {
    result += teammate(iPokemon).isAlive();
  }
  
  return result;
}


bool TeamVolatile::swapPokemon(size_t iAction, bool preserveVolatile)
{
  size_t iOldPokemon = getICPKV();
  size_t iNewPokemon = iAction - AT_SWITCH_0;

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
    os << linePrefix << iTeammate << "-" << teammate(iTeammate);
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