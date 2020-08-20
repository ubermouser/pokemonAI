#include "../inc/team_nonvolatile.h"

#include <stdexcept>

#include "../inc/pokemon_base.h"
#include "../inc/team_volatile.h"
#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

using namespace orphan;


TeamNonVolatile::TeamNonVolatile()
  : Name(), 
  Signature<TeamNonVolatile, TEAM_NONVOLATILE_DIGESTSIZE>(),
  teammates(),
  numTeammates(0)
{
}

TeamNonVolatile::TeamNonVolatile(const TeamNonVolatile& orig)
  : Name(orig), 
  Signature<TeamNonVolatile, TEAM_NONVOLATILE_DIGESTSIZE>(orig),
  teammates(orig.teammates),
  numTeammates(orig.numTeammates)
{ 
}





PokemonNonVolatile& TeamNonVolatile::teammate(size_t iTeammate)
{
  assert(iTeammate < getNumTeammates());
  return teammates[iTeammate];
};





const PokemonNonVolatile& TeamNonVolatile::teammate(size_t iTeammate) const
{
  assert(iTeammate < getNumTeammates());
  return teammates[iTeammate];
};





const PokemonNonVolatile& TeamNonVolatile::getPKNV(const TeamVolatile& source) const
{
  return teammates[source.getICPKV()];
};




TeamNonVolatile& TeamNonVolatile::addPokemon(const PokemonNonVolatile& cPokemon)
{
  assert(isLegalAdd(cPokemon));

  teammates[numTeammates] = cPokemon;
  numTeammates++;
  return *this;
}





TeamNonVolatile& TeamNonVolatile::setPokemon(size_t iPokemon, const PokemonNonVolatile& swappedPokemon)
{
  assert(isLegalSet(iPokemon, swappedPokemon));
  teammate(iPokemon) = swappedPokemon;
  return *this;
};





TeamNonVolatile& TeamNonVolatile::setLeadPokemon(size_t iTeammate)
{
  assert(iTeammate < getNumTeammates());

  // asked to switch lead pokemon with lead pokemon
  if (iTeammate == 0) return *this;

  PokemonNonVolatile switchedPokemon = teammate(0);
  teammate(0) = teammate(iTeammate);
  teammate(iTeammate) = switchedPokemon;
  return *this;
}





bool TeamNonVolatile::isLegalAdd(const PokemonNonVolatile& candidate) const
{
  if ( !candidate.pokemonExists() ) { return false; }
  return isLegalAdd(candidate.getBase());
}






bool TeamNonVolatile::isLegalSet(size_t iPosition, const PokemonNonVolatile& candidate) const
{
  if ( !candidate.pokemonExists() ) { return false; }
  return isLegalSet(iPosition, candidate.getBase());
}





bool TeamNonVolatile::isLegalAdd(const PokemonBase& candidate) const
{
  if ((getNumTeammates() + 1) > getMaxNumTeammates()) { return false; }
  return isLegalSet(SIZE_MAX, candidate);
}






bool TeamNonVolatile::isLegalSet(size_t iPosition, const PokemonBase& candidate) const
{
  if ((iPosition != SIZE_MAX) && (iPosition >= getNumTeammates()) ) { return false; }
  if (candidate.lostChild_ == true) { return false; }
  for (size_t iTeammate = 0; iTeammate != getNumTeammates(); ++iTeammate)
  {
    if (iPosition == iTeammate) { continue; }
    if (&teammate(iTeammate).getBase() == &candidate) { return false; }
  }

  return true;
}





TeamNonVolatile& TeamNonVolatile::removePokemon(size_t iRemovedPokemon)
{
  // don't bother removing a pokemon that doesn't exist
  if (iRemovedPokemon >= getNumTeammates()) { return *this; }

  {
    PokemonNonVolatile& removedPokemon = teammate(iRemovedPokemon);

    // remove pokemon:
    removedPokemon = PokemonNonVolatile();
  }

  // there's a "hole" in the contiguous teammate array now, so 
  // refactor pokemon above the removed pokemon:
  for (size_t iSource = iRemovedPokemon + 1; iSource < getNumTeammates(); iSource++)
  {
    PokemonNonVolatile& source = teammate(iSource);

    for (size_t iNDestination = 0; iNDestination < (iRemovedPokemon + 1); iNDestination++)
    {
      size_t iDestination = (iSource - iNDestination - 1);

      PokemonNonVolatile& destination = teammates[iDestination];

      // don't replace a pokemon that exists already
      if (destination.pokemonExists()) { continue; }

      // perform copy
      destination = source;
      // delete source (no duplicates)
      source = PokemonNonVolatile();
      break;
    }
  }

  // update number of teammates in teammate array:
  numTeammates--;
  return *this;
}





TeamNonVolatile& TeamNonVolatile::initialize()
{
  for (size_t iPokemon = 0; iPokemon < getNumTeammates(); iPokemon++)
  {
    teammate(iPokemon).initialize();
  }
  return *this;
};





void TeamNonVolatile::uninitialize()
{
  for (size_t iPokemon = 0; iPokemon < getNumTeammates(); iPokemon++)
  {
    teammate(iPokemon).uninitialize();
  }
};





void TeamNonVolatile::createDigest_impl(std::array<uint8_t, TEAM_NONVOLATILE_DIGESTSIZE>& digest) const
{
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
  for (size_t iOrder = 1, iSize = getNumTeammates(); iOrder < iSize; ++iOrder)
  {
    size_t iBestPokemon = SIZE_MAX;
    const PokemonNonVolatile* bestPokemon = NULL;
    for (size_t iPokemon = 0; iPokemon != iSize; ++iPokemon)
    {
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
  pack(numTeammates, digest, iDigest);

  assert(iDigest == TEAM_NONVOLATILE_DIGESTSIZE);
}



static const std::string header = "PKAIE0";

void TeamNonVolatile::output(std::ostream& oFile, bool printHeader) const
{
  // header:
  if (printHeader)
  {
    oFile << header << "\t";
  }

  // output team name:
  oFile << getName() << "\t";
  // output team size:
  oFile << getNumTeammates() << "\n";
  // output team pokemon:
  for (size_t iTeammate = 0; iTeammate != getNumTeammates(); ++iTeammate)
  {
    const PokemonNonVolatile& cTeammate = teammate(iTeammate);
    cTeammate.output(oFile);
  }
} // endOf outputTeam

bool TeamNonVolatile::input(const std::vector<std::string>& lines, size_t& iLine)
{
  /*
   * Header data:
   * PKAIE0 <team name> <numTeammates>\n
   * <nickname> <species> <level> <item> <gender> <ability> <nature> <hp.type> <hp.dmg> <move 1> <move 2> <move 3> <move 4> <atk.iv> <spatck.iv> <def.iv> <spdef.iv> <spd.iv> <hp.iv> <atk.ev> <spatck.ev> <def.ev> <spdef.ev> <spd.ev> <hp.ev>
   */

  OrphanSet mismatchedPokemon;
  OrphanSet mismatchedItems;
  OrphanSet mismatchedAbilities;
  OrphanSet mismatchedNatures;
  OrphanSet mismatchedMoves;

  // are the enough lines in the input stream:
  if ((lines.size() - iLine) < 2U)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": unexpected end of input stream at line " << iLine << "!\n";
    return false; 
  }

  // compare team_nonvolatile header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": team_nonvolatile stream has header of type \"" << lines.at(0).substr(0, header.size()) << 
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // guess size of teammate array, input name:
  size_t _numTeammates;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)3, (size_t)3)) { return false; }

    if (!INI::setArgAndPrintError("team_nonvolatile numTeammates", tokens.at(2), _numTeammates, iLine, 2)) { return false; }

    if (!INI::checkRangeB(_numTeammates, (size_t)1, getMaxNumTeammates())) { return false; }
    // maximum name size is 20 chars
    setName(tokens.at(1).substr(0, 20));
  }

  iLine++;
  for (size_t iTeammate = 0; iTeammate < _numTeammates; ++iTeammate)
  {
    PokemonNonVolatile cTeammate;
    if (!cTeammate.input(lines, iLine, &mismatchedPokemon, &mismatchedItems, &mismatchedAbilities, &mismatchedNatures, &mismatchedMoves)) { return false; }

    // check for duplicate teammate names:
    {
      // linear search, because we're comparing 5 or less and they're not sorted
      for (size_t iPTeammate = 0; iPTeammate < getNumTeammates(); iPTeammate++)
      {
        if (cTeammate.getName().compare(teammate(iPTeammate).getName()) == 0)
        {
          std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
            ": Pokemon " << iPTeammate << 
            " has same nickname as pokemon " << iTeammate <<
            "-\"" << cTeammate.getName() << "\"!\n";
          return false;
        }
      }
    }
#ifndef _ALLOWINVALIDTEAMS
    // if this pokemon may not be added to the team:
    if (!isLegalAdd(cTeammate)) { continue; }
#endif
    // add the teammate to team array:
    addPokemon(cTeammate);
  } //end of per-teammate

  bool result = true;

  // determine that the number of teams is adequate:
  if (getNumTeammates() == 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": team \"" << getName() <<
      " does not have enough valid pokemon (" << getNumTeammates() <<
      ")!\n";
    result = false;
  }

  // print mismatched pokemon
  printOrphans(mismatchedPokemon, getName(), "team-pokemon", "pokemon", 4);

  // print mismatched items
  printOrphans(mismatchedItems, getName(), "team-items", "item", 4);

  // print mismatched abilities
  printOrphans(mismatchedAbilities, getName(), "team-abilities", "ability", 4);

  // print mismatched natures
  printOrphans(mismatchedNatures, getName(), "team-natures", "nature", 4);

  // print mismatched moves
  printOrphans(mismatchedMoves, getName(), "team-moves", "move", 4);
  return result; // import success or failure, depending on if we have orphans
} // endof import team


TeamNonVolatile TeamNonVolatile::loadFromFile(const std::string& path) {
  TeamNonVolatile result;
  size_t iLine = 0;
  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (INI::loadFileToString(path, "PKAIE", inputBuffer) != true) {
      throw std::invalid_argument("TeamNonVolatile read failure");
    }

    //tokenize by line endings
    lines = INI::tokenize(inputBuffer, "\n\r");
  }

  // this team object does not have any built-in ranking data, and should be easier to load:
  if (lines.at(iLine).compare(0, 5, "PKAIE") == 0)
  {
    // input actual team:
    if (!result.input(lines, iLine)) { throw std::invalid_argument("TeamNonVolatile deserialization failure"); }
  } else {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": File \"" << path << "\" has header of type \"" << lines.at(0).substr(0, 5) <<
      "\" (needs to be \"PKAIE\" or \"PKAIR\") and is incompatible with this program!\n";
    throw std::invalid_argument("File header does not match PKAIE");
  }

  return result;
} // endOf loadFromFile
