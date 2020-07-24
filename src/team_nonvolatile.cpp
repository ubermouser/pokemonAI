
//#define PKAI_EXPORT
#include "../inc/team_nonvolatile.h"

//#define PKAI_STATIC
#include "../inc/pokemon_base.h"
#include "../inc/team_volatile.h"
//#undef PKAI_STATIC
#include "../inc/init_toolbox.h"

team_nonvolatile::team_nonvolatile()
  : name(), 
  signature<team_nonvolatile, TEAM_NONVOLATILE_DIGESTSIZE>(),
  teammates(),
  numTeammates(0)
{
}

team_nonvolatile::team_nonvolatile(const team_nonvolatile& orig)
  : name(orig), 
  signature<team_nonvolatile, TEAM_NONVOLATILE_DIGESTSIZE>(orig),
  teammates(orig.teammates),
  numTeammates(orig.numTeammates)
{ 
}





pokemon_nonvolatile& team_nonvolatile::teammate(size_t iTeammate)
{
  assert(iTeammate < getNumTeammates());
  return teammates[iTeammate];
};





const pokemon_nonvolatile& team_nonvolatile::teammate(size_t iTeammate) const
{
  assert(iTeammate < getNumTeammates());
  return teammates[iTeammate];
};





const pokemon_nonvolatile& team_nonvolatile::getPKNV(const team_volatile& source) const
{
  return teammates[source.getICPKV()];
};




void team_nonvolatile::addPokemon(const pokemon_nonvolatile& cPokemon)
{
  assert(isLegalAdd(cPokemon));

  teammates[numTeammates] = cPokemon;
  numTeammates++;
}





void team_nonvolatile::setPokemon(size_t iPokemon, const pokemon_nonvolatile& swappedPokemon)
{
  assert(isLegalSet(iPokemon, swappedPokemon));
  teammate(iPokemon) = swappedPokemon;
};





void team_nonvolatile::setLeadPokemon(size_t iTeammate)
{
  assert(iTeammate < getNumTeammates());

  // asked to switch lead pokemon with lead pokemon
  if (iTeammate == 0) return;

  pokemon_nonvolatile switchedPokemon = teammate(0);
  teammate(0) = teammate(iTeammate);
  teammate(iTeammate) = switchedPokemon;
}





bool team_nonvolatile::isLegalAdd(const pokemon_nonvolatile& candidate) const
{
  if ( !candidate.pokemonExists() ) { return false; }
  return isLegalAdd(candidate.getBase());
}






bool team_nonvolatile::isLegalSet(size_t iPosition, const pokemon_nonvolatile& candidate) const
{
  if ( !candidate.pokemonExists() ) { return false; }
  return isLegalSet(iPosition, candidate.getBase());
}





bool team_nonvolatile::isLegalAdd(const pokemon_base& candidate) const
{
  if ((getNumTeammates() + 1) > getMaxNumTeammates()) { return false; }
  return isLegalSet(SIZE_MAX, candidate);
}






bool team_nonvolatile::isLegalSet(size_t iPosition, const pokemon_base& candidate) const
{
  if ((iPosition != SIZE_MAX) && (iPosition >= getNumTeammates()) ) { return false; }
  if (candidate.lostChild == true) { return false; }
  for (size_t iTeammate = 0; iTeammate != getNumTeammates(); ++iTeammate)
  {
    if (iPosition == iTeammate) { continue; }
    if (&teammate(iTeammate).getBase() == &candidate) { return false; }
  }

  return true;
}





void team_nonvolatile::removePokemon(size_t iRemovedPokemon)
{
  // don't bother removing a pokemon that doesn't exist
  if (iRemovedPokemon >= getNumTeammates()) { return; }

  {
    pokemon_nonvolatile& removedPokemon = teammate(iRemovedPokemon);

    // remove pokemon:
    removedPokemon = pokemon_nonvolatile();
  }

  // there's a "hole" in the contiguous teammate array now, so 
  // refactor pokemon above the removed pokemon:
  for (size_t iSource = iRemovedPokemon + 1; iSource < getNumTeammates(); iSource++)
  {
    pokemon_nonvolatile& source = teammate(iSource);

    for (size_t iNDestination = 0; iNDestination < (iRemovedPokemon + 1); iNDestination++)
    {
      size_t iDestination = (iSource - iNDestination - 1);

      pokemon_nonvolatile& destination = teammates[iDestination];

      // don't replace a pokemon that exists already
      if (destination.pokemonExists()) { continue; }

      // perform copy
      destination = source;
      // delete source (no duplicates)
      source = pokemon_nonvolatile();
      break;
    }
  }

  // update number of teammates in teammate array:
  numTeammates--;
}





void team_nonvolatile::initialize()
{
  for (size_t iPokemon = 0; iPokemon < getNumTeammates(); iPokemon++)
  {
    teammate(iPokemon).initialize();
  }
};





void team_nonvolatile::uninitialize()
{
  for (size_t iPokemon = 0; iPokemon < getNumTeammates(); iPokemon++)
  {
    teammate(iPokemon).uninitialize();
  }
};





void team_nonvolatile::createDigest_impl(boost::array<uint8_t, TEAM_NONVOLATILE_DIGESTSIZE>& digest) const
{
  digest.assign(0);

  boost::array<bool, 6> packedPokemon;
  packedPokemon.assign(false);
  size_t iDigest = 0;

  // do NOT pack the name of the team:

  // always hash the first pokemon first:
  {
    const pokemon_nonvolatile& firstPokemon = teammate(0);
    packedPokemon[0] = true;

    // pack pokemon:
    boost::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE> fPokemonDigest;
    firstPokemon.createDigest(fPokemonDigest);

    // copy action to pokemon digest:
    pack(fPokemonDigest, digest, iDigest);
  }

  // hash by a useful order:
  for (size_t iOrder = 1, iSize = getNumTeammates(); iOrder < iSize; ++iOrder)
  {
    size_t iBestPokemon = SIZE_MAX;
    const pokemon_nonvolatile* bestPokemon = NULL;
    for (size_t iPokemon = 0; iPokemon != iSize; ++iPokemon)
    {
      const pokemon_nonvolatile& cPokemon = teammate(iPokemon);

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
    boost::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE> bPokemonDigest;
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

void team_nonvolatile::output(std::ostream& oFile, bool printHeader) const
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
    const pokemon_nonvolatile& cTeammate = teammate(iTeammate);
    cTeammate.output(oFile);
  }
} // endOf outputTeam

bool team_nonvolatile::input(const std::vector<std::string>& lines, size_t& iLine)
{
  /*
   * Header data:
   * PKAIE0 <team name> <numTeammates>\n
   * <nickname> <species> <level> <item> <gender> <ability> <nature> <hp.type> <hp.dmg> <move 1> <move 2> <move 3> <move 4> <atk.iv> <spatck.iv> <def.iv> <spdef.iv> <spd.iv> <hp.iv> <atk.ev> <spatck.ev> <def.ev> <spdef.ev> <spd.ev> <hp.ev>
   */

  std::vector<std::string> mismatchedPokemon;
  std::vector<std::string> mismatchedItems;
  std::vector<std::string> mismatchedAbilities;
  std::vector<std::string> mismatchedNatures;
  std::vector<std::string> mismatchedMoves;

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
    pokemon_nonvolatile cTeammate;
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
  if (mismatchedPokemon.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << getName() <<
      "\" - " << mismatchedPokemon.size() << " Orphaned team-pokemon!\n";
    if (verbose >= 4)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedPokemon.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned pokemon \"" << mismatchedPokemon.at(iOrphan) << "\"\n";
      }
    }
    //result = false;
  }

  // print mismatched items
  if (mismatchedItems.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << getName() <<
      "\" - " << mismatchedItems.size() << " Orphaned team-items!\n";
    if (verbose >= 4)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedItems.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned item \"" << mismatchedItems.at(iOrphan) << "\"\n";
      }
    }
    //result = false;
  }

  // print mismatched abilities
  if (mismatchedAbilities.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << getName() <<
      "\" - " << mismatchedAbilities.size() << " Orphaned team-abilities!\n";
    if (verbose >= 4)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedAbilities.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned ability \"" << mismatchedAbilities.at(iOrphan) << "\"\n";
      }
    }
    //result = false;
  }

  // print mismatched natures
  if (mismatchedNatures.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << getName() <<
      "\" - " << mismatchedNatures.size() << " Orphaned team-natures!\n";
    if (verbose >= 4)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedNatures.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned nature \"" << mismatchedNatures.at(iOrphan) << "\"\n";
      }
    }
    //result = false;
  }

  // print mismatched moves
  if (mismatchedMoves.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << getName() <<
      "\" - " << mismatchedMoves.size() << " Orphaned team-moves!\n";
    if (verbose >= 4)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedMoves.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned move \"" << mismatchedMoves.at(iOrphan) << "\"\n";
      }
    }
    //result = false;
  }
  return result; // import success or failure, depending on if we have orphans
} // endof import team