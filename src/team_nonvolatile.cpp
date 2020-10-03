#include "../inc/team_nonvolatile.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>

#include "../inc/pokemon_base.h"
#include "../inc/team_volatile.h"
#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

using namespace orphan;
namespace pt = boost::property_tree;


TeamNonVolatile::TeamNonVolatile()
  : Name(),
  Signature<TeamNonVolatile, TEAM_NONVOLATILE_DIGESTSIZE>(),
  Serializable<TeamNonVolatile>(),
  teammates_() {
}


PokemonNonVolatile& TeamNonVolatile::teammate(size_t iTeammate) {
  return teammates_.at(iTeammate);
};


const PokemonNonVolatile& TeamNonVolatile::teammate(size_t iTeammate) const {
  assert(iTeammate < getNumTeammates());
  return teammates_[iTeammate];
};


const PokemonNonVolatile& TeamNonVolatile::getPKNV(const TeamVolatile& source) const {
  return teammates_[source.getICPKV()];
};


TeamNonVolatile& TeamNonVolatile::addPokemon(const PokemonNonVolatile& cPokemon) {
  if (!isLegalAdd(cPokemon)) { throw std::invalid_argument("TeamNonVolatile illegal pokemon"); }

  teammates_.push_back(cPokemon);
  return *this;
}


TeamNonVolatile& TeamNonVolatile::setPokemon(size_t iPokemon, const PokemonNonVolatile& swappedPokemon) {
  if (isLegalSet(iPokemon, swappedPokemon)) {
    throw std::invalid_argument("TeamNonVolatile illegal pokemon");
  }

  teammate(iPokemon) = swappedPokemon;
  return *this;
};


TeamNonVolatile& TeamNonVolatile::setLeadPokemon(size_t iTeammate) {
  assert(iTeammate < getNumTeammates());

  // asked to switch lead pokemon with lead pokemon
  if (iTeammate == 0) return *this;

  std::swap(teammate(0), teammate(iTeammate));
  return *this;
}


bool TeamNonVolatile::isLegalAdd(const PokemonNonVolatile& candidate) const {
  if ( !candidate.pokemonExists() ) { return false; }
  return isLegalAdd(candidate.getBase());
}


bool TeamNonVolatile::isLegalSet(size_t iPosition, const PokemonNonVolatile& candidate) const {
  if ( !candidate.pokemonExists() ) { return false; }
  return isLegalSet(iPosition, candidate.getBase());
}


bool TeamNonVolatile::isLegalAdd(const PokemonBase& candidate) const {
  if ((getNumTeammates() + 1) > getMaxNumTeammates()) { return false; }
  return isLegalSet(SIZE_MAX, candidate);
}


bool TeamNonVolatile::isLegalSet(size_t iPosition, const PokemonBase& candidate) const {
  if ((iPosition != SIZE_MAX) && (iPosition >= getNumTeammates()) ) { return false; }
  if (candidate.lostChild_ == true) { return false; }
  for (size_t iTeammate = 0; iTeammate != getNumTeammates(); ++iTeammate)
  {
    if (iPosition == iTeammate) { continue; }
    if (&teammate(iTeammate).getBase() == &candidate) { return false; }
  }

  return true;
}


TeamNonVolatile& TeamNonVolatile::removePokemon(size_t iRemovedPokemon) {
  // don't bother removing a pokemon that doesn't exist
  if (iRemovedPokemon >= getNumTeammates()) { return *this; }

  teammates_.erase(teammates_.begin() + iRemovedPokemon);
  return *this;
}


TeamNonVolatile& TeamNonVolatile::initialize() {
  for (auto& pkmn : teammates_) {
    pkmn.initialize();
  }
  return *this;
};


void TeamNonVolatile::uninitialize() {
  for (auto& pkmn : teammates_) {
    pkmn.uninitialize();
  }
};


void TeamNonVolatile::createDigest_impl(std::array<uint8_t, TEAM_NONVOLATILE_DIGESTSIZE>& digest) const {
  digest.fill(0);

  std::array<bool, 6> packedPokemon;
  packedPokemon.fill(false);
  size_t iDigest = 0;

  // do NOT pack the name of the team:

  // always hash the first pokemon first:
  {
    const PokemonNonVolatile& firstPokemon = teammate(0);
    packedPokemon[0] = true;

    // pack pokemon:
    std::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE> fPokemonDigest;
    firstPokemon.createDigest(fPokemonDigest);

    // copy action to pokemon digest:
    pack(fPokemonDigest, digest, iDigest);
  }

  // hash by a useful order:
  for (size_t iOrder = 1, iSize = getNumTeammates(); iOrder < iSize; ++iOrder) {
    size_t iBestPokemon = SIZE_MAX;
    const PokemonNonVolatile* bestPokemon = NULL;
    for (size_t iPokemon = 0; iPokemon != iSize; ++iPokemon) {
      const PokemonNonVolatile& cPokemon = teammate(iPokemon);

      // don't pack a pokemon that has already been packed:
      if (packedPokemon[iPokemon] == true) { continue; }

      // if no move has been selected yet, select the first move:
      if (bestPokemon == NULL) { bestPokemon = &cPokemon; iBestPokemon = iPokemon; continue; }

      // if bestMove appears later in the array of base moves than does cMove: (higher in alphabetical order)
      if (&bestPokemon->getBase() > &cPokemon.getBase()) { bestPokemon = &cPokemon; iBestPokemon = iPokemon; }
    }

    // no more pokemon to be hashed
    if (bestPokemon == NULL) { break; }

    packedPokemon[iBestPokemon] = true;

    // pack pokemon:
    std::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE> bPokemonDigest;
    bestPokemon->createDigest(bPokemonDigest);

    // copy action to pokemon digest:
    pack(bPokemonDigest, digest, iDigest);
  } // endOf foreach moveOrdered
  iDigest = POKEMON_NONVOLATILE_DIGESTSIZE * 6;

  // pack number of pokemon in team:
  pack(uint8_t(getNumTeammates()), digest, iDigest);

  assert(iDigest == TEAM_NONVOLATILE_DIGESTSIZE);
}


const std::string& TeamNonVolatile::defineName() {
  std::ostringstream os;
  size_t numPokemon = getNumTeammates();
  os << boost::format("%d-x%06x_") % numPokemon % (hash() & 0xffffff);

  // print a few characters from each teammate:
  size_t teammateStringSize = 0;
  size_t sizeAccumulator = 0;
  for (size_t iTeammate = 0; iTeammate != getNumTeammates(); ++iTeammate) {
    const auto& pknv = teammate(iTeammate);
    teammateStringSize = (24 - sizeAccumulator) / (getNumTeammates() - iTeammate);

    std::string basePrefix = pknv.getBase().getName().substr(0, teammateStringSize);
    if (basePrefix.size() > 0) { basePrefix[0] = std::toupper(basePrefix[0]); }
    os << basePrefix;

    sizeAccumulator += basePrefix.size();
  }
  setName(os.str());
  return getName();
}


std::ostream& operator <<(std::ostream& os, const TeamNonVolatile& tNV) {
  for (size_t iTeammate = 0; iTeammate != tNV.getNumTeammates(); ++iTeammate) {
    os << iTeammate << "-" << tNV.teammate(iTeammate) << "\n";
  }
  return os;
}


void TeamNonVolatile::printSummary(std::ostream& os, const std::string& prefix) const {
  for (size_t iTeammate = 0; iTeammate != getNumTeammates(); ++iTeammate) {
    os << prefix << iTeammate << "-";
    teammate(iTeammate).printSummary(os);
    os << "\n";
  }
}


static const std::string header = "PKAIE0";


boost::property_tree::ptree TeamNonVolatile::output(bool printHeader) const {
  pt::ptree result;
  if (printHeader) {
    result.put("header", header);
  }
  result.put("name", getName());

  pt::ptree& pkmn = result.put_child("pokemon", pt::ptree{});
  for (size_t iPokemon = 0; iPokemon < getNumTeammates(); ++iPokemon) {
    pkmn.push_back(pt::ptree::value_type{"", teammate(iPokemon).output(printHeader)});
  }

  return result;
}


void TeamNonVolatile::input(const pt::ptree& ptree) {
  // if we were provided a container class of team, ignore the container component
  if (ptree.count("team") > 0) { return input(ptree.get_child("team")); }

  Orphanage orphans;

  setName(ptree.get<std::string>("name"));
  for (auto& e: ptree.get_child("pokemon")) {
    PokemonNonVolatile cTeammate;
    cTeammate.input(e.second, orphans);

    // add the teammate to team array:
    addPokemon(cTeammate);
  }

  // determine that the number of teams is adequate:
  if (getNumTeammates() == 0) {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": team \"" << getName() <<
      " does not have enough valid pokemon (" << getNumTeammates() <<
      ")!\n";

    throw std::invalid_argument("TeamNonVolatile numTeammates");
  }

  orphans.printAllOrphans(getName(), "team", 4);
}
