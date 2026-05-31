#include "pokemonai/team_volatile.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <bit>
#include <bitset>
#include <boost/static_assert.hpp>
#include <cstring>
#include <ostream>

#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/team_nonvolatile.h"

BOOST_STATIC_ASSERT(sizeof(VolatileStatus) == (sizeof(uint32_t)*4));
BOOST_STATIC_ASSERT(sizeof(TeamStatus) == (sizeof(uint32_t) * 1));
BOOST_STATIC_ASSERT(sizeof(TeamVolatileData) == (sizeof(uint32_t) * 25));

bool TeamVolatileData::operator==(const TeamVolatileData& other) const {
  return std::memcmp(this, &other, sizeof(TeamVolatileData)) == 0;
}

bool TeamVolatileData::operator !=(const TeamVolatileData& other) const
{
  return !(*this == other);
}


void TeamVolatile::initialize(size_t numActivePokemon) {
  // status and all other variables have been zeroed from a different context
  for (size_t iTeammate = 0, _numTeammates = nv_->getNumTeammates(); iTeammate != _numTeammates; ++iTeammate)
  {
    // 0th teammate is active at the start of the game
    size_t iActive = iTeammate < numActivePokemon ? iTeammate + 1 : 0;
    teammate(iTeammate).initialize(iActive);
  }
}


TEAM_VOLATILE_IMPL_TEMPLATE
typename TEAM_VOLATILE_IMPL::pokemonvolatile_t
TEAM_VOLATILE_IMPL::teammate(size_t iTeammate) const {
  auto& teammateData = data().teammates[iTeammate];
  size_t activeSlot = teammateData.active;
  auto& activeVol = activeSlot > 0 ? data().activeVolatiles[activeSlot - 1]
                                   : data().activeVolatiles[0];
  return pokemonvolatile_t{
      nv().teammate(iTeammate),  // contains assertion
      teammateData,
      const_cast<typename pokemonvolatile_t::status_t&>(activeVol)};
};


TEAM_VOLATILE_IMPL_TEMPLATE
std::vector<Actor> TEAM_VOLATILE_IMPL::getActiveActors() const {
  std::vector<Actor> result;
  for (const Actor& actor : yieldActiveActors()) { result.push_back(actor); }
  return result;
}


TEAM_VOLATILE_IMPL_TEMPLATE
size_t TEAM_VOLATILE_IMPL::getNumActivePokemon() const {
  size_t result = 0;
  for (const Actor& actor : yieldActiveActors()) {
    (void)actor;
    ++result;
  }
  return result;
}


TEAM_VOLATILE_IMPL_TEMPLATE
uint32_t TEAM_VOLATILE_IMPL::numTeammatesAlive() const {
  uint32_t result = 0; // accumulate living teammates
  for (const auto& [actor, pkmn] : yieldPokemon()) { result += pkmn.isAlive(); }

  return result;
}


TEAM_VOLATILE_IMPL_TEMPLATE
bool TEAM_VOLATILE_IMPL::isAlive() const {
  for (const auto& [actor, pkmn] : yieldPokemon()) {
    if (pkmn.isAlive()) { return true; }
  }

  return false;
}


bool TeamVolatile::swapPokemon(size_t iAction, bool preserveVolatile) {
  return swapPokemon(
      Actor{nv().iTeam(), getICPKV()},
      Actor{nv().iTeam(), iAction},
      preserveVolatile);
}


bool TeamVolatile::swapPokemon(
    const Actor& actor, const Actor& target, bool preserveVolatile) {
  assert(actor.iTeam() == nv().iTeam());
  assert(target.iTeam() == nv().iTeam());

  // make sure we're not switching to ourself
  if (actor == target) { return false; }

  // rewrite swap pokemon value:
  auto oPkmn = teammate(actor);
  auto nPkmn = teammate(target);

  assert(oPkmn.isActive());

  auto oPkmnActive = oPkmn.data().active;
  assert(oPkmnActive > 0 && oPkmnActive <= 3);

  // reset the volatile status for the slot we are switching out of:
  if (!preserveVolatile) {
    data_->activeVolatiles[oPkmnActive - 1] = VolatileStatus();
  }

  // cannot use std::swap because this is a bitfield
  auto nPkmnActive = nPkmn.data().active;
  nPkmn.data().active = oPkmn.data().active;
  oPkmn.data().active = nPkmnActive;

  return true;
}


size_t TeamVolatile::activatePokemon(const Actor& actor) {
  assert(actor.iTeam() == nv().iTeam());
  auto pkmn = teammate(actor);
  assert(!pkmn.isActive());

  // find the first empty slot for the active pokemon to enter:
  std::bitset<6> occupied;
  for (const auto& [actor, pkmn] : yieldActivePokemon()) {
    occupied.set(pkmn.data().active - 1);
  }

  size_t firstEmptySlot = std::countl_one(occupied.to_ulong());
  pkmn.data().active = (uint8_t)firstEmptySlot + 1;

  // reset volatile status in this newly active slot:
  data_->activeVolatiles[firstEmptySlot] = VolatileStatus();

  return firstEmptySlot;
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


std::ostream& operator <<(std::ostream& os, const ConstTeamVolatile& team) {
  team.printTeam(os);
  const auto& s = team.status();
  if (s.spikes > 0) { os << fmt::format(" (SPIKES-{})", s.spikes); }
  if (s.stealthRock > 0) { os << " (STLTH_ROCK)"; }
  if (s.toxicSpikes > 0) { os << fmt::format(" (T-SPIKES-{})", s.toxicSpikes); }
  if (s.lightScreen > 0) { os << fmt::format(" (L-SCRN-{})", s.lightScreen); }
  if (s.reflect > 0) { os << fmt::format(" (REFLECT-{})", s.reflect); }
  return os;
}


template class TeamVolatileImpl<ConstPokemonVolatile, const TeamVolatileData, const TeamStatus>;
template class TeamVolatileImpl<PokemonVolatile, TeamVolatileData, TeamStatus>;