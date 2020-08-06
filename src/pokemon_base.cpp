#include "../inc/pokemon_base.h"

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

#include "../inc/ability.h"
#include "../inc/type.h"
#include "../inc/move.h"

using namespace INI;
using namespace orphan;

const PokemonBase* PokemonBase::no_base = NULL;


bool Pokemons::initialize(
    const std::string& pokemonPath,
    const std::string& movelistPath,
    const Types& types,
    const Abilities& abilities,
    const Moves& moves) {
  if (pokemonPath.empty())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": A species list has not been defined!\n";
    return false;
  }
  if (verbose >= 1) std::cout << " Loading species library...\n";
  if (!loadFromFile(pokemonPath, types, abilities))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": inputPokemon failed to populate a list of pokemon.\n";
    return false;
  }
  std::sort(begin(), end());

  {
    // find special case no base:
    PokemonBase::no_base = orphan::orphanCheck_ptr(*this, NULL, "none");
  }

  //MOVELIST library (requires sorted input of pokemon and moves!)
  if (movelistPath.empty())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": A movelist array has not been defined!\n";
    return false;
  }
  if (verbose >= 1) std::cout << " Loading Pokemon move arrays...\n";
  if (!loadMovelistFromFile(movelistPath, moves))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": inputMoveList failed to populate an acceptable list of pokemon moves.\n";
    return false;
  }
  
  return true;
}


bool Pokemons::loadFromFile(
    const std::string& path, const Types& types, const Abilities& abilities) {
  /*
   * Header data:
   * PKAIS <SIZE> #fluff\n
   * #fluff line\n
   * <String NAME>\t<float 1>\t<float 2>\t<float ... SIZE>
   */

  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAIS", inputBuffer) != true)
    {
      return false;
    }

    //tokenize by line endings
    lines = tokenize(inputBuffer, "\n\r");
  }

  size_t iLine = 0;
  bool result = loadFromFile_lines(types, abilities, lines, iLine);
  assert(iLine == lines.size());
  return result && (iLine == lines.size());
} // endof import pokemon


bool Pokemons::loadFromFile_lines(
    const Types& types,
    const Abilities& abilities,
    const std::vector<std::string>& lines,
    size_t& iLine) {
  static const std::string header = "PKAIS";
  /*
   * Header data:
   * PKAIS <SIZE> #fluff\n
   * #fluff line\n
   * <String NAME>\t<float 1>\t<float 2>\t<float ... SIZE>
   */

  // are the enough lines in the input stream for at least the header:
  if ((lines.size() - iLine) < 2U)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": unexpected end of input stream at line " << iLine << "!\n";
    return false;
  }

  // compare header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": pokemon inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) <<
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // guess size of array:
  size_t _numElements;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

    if (!INI::setArgAndPrintError("pokemon numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

    if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXPOKEMON)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXPOKEMON)) { return false; }
  resize(_numElements);

  std::vector<std::string> mismatchedTypes;
  std::vector<std::string> mismatchedAbilities;

  for (size_t iPokemon = 0; iPokemon < size(); ++iPokemon, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    class PokemonBase& cPokemon = at(iPokemon);
    if (tokens.size() != 12)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": pokemon inputStream has malformed line #" << (iLine) <<
        " with " << tokens.size() << " values!\n";
      return false;
    }
    // init lostChild
    cPokemon.lostChild_ = false;

    //pokemon name
    size_t iToken = 0;
    cPokemon.setName(tokens.at(iToken));

    //pokemon primary type
    iToken = 1;
    {
      size_t iType = orphanCheck(types, &mismatchedTypes, tokens.at(iToken));
      if (iType == SIZE_MAX) { cPokemon.lostChild_ = true; } //orphan!
      cPokemon.types_[0] = &types[iType];
    }

    //pokemon secondary type
    iToken = 2;
    {
      size_t iType = orphanCheck(types, &mismatchedTypes, tokens.at(iToken));
      if (iType == SIZE_MAX) { cPokemon.lostChild_ = true; } //orphan!
      cPokemon.types_[1] = &types[iType]; // notype = "None"
    }

    //base stats table
    iToken = 3;
    for (size_t iStat = 0; iStat < 6; iStat++)
    {
      uint32_t cStat;
      if (!setArg(tokens.at(iToken + iStat), cStat)) { incorrectArgs("cStat", iLine, iToken + iStat); return false; }
      checkRangeB(cStat, (uint32_t)1, (uint32_t)255);
      cPokemon.baseStats_[iStat] = cStat;
    }

    //weight
    iToken = 9;
    {
      fpType cWeight;
      if (!setArg(tokens.at(iToken), cWeight)) { incorrectArgs("cWeight", iLine, iToken); return false; }
      checkRangeB(cWeight, (fpType)0.1, (fpType)1000.0);
      cPokemon.weight_ = cWeight * WEIGHTMULTIPLIER;
    }

    //first ability choice
    iToken = 10;
    {
      const Ability* cAbility = orphanCheck_ptr(abilities, &mismatchedAbilities, tokens.at(iToken));
      if (cAbility == NULL) { cPokemon.lostChild_ = true; } //orphan!
      else { cPokemon.abilities_.push_back(cAbility); }
    }

    //second ability choice
    iToken = 11;
    if (tokens.at(iToken).compare("---") != 0)
    {
      //strncpy(currentPokemon->secondaryAbilityName,tokens.at(11),20);
      const Ability* cAbility = orphanCheck_ptr(abilities, &mismatchedAbilities, tokens.at(iToken));
      if (cAbility == NULL) { cPokemon.lostChild_ = true; } //orphan!
      else { cPokemon.abilities_.push_back(cAbility); }
    }

    // sort abilities (by pointer):
    {
      std::sort(cPokemon.abilities_.begin(), cPokemon.abilities_.end());
    }

  } //end of per-pokemon

  //output orphans
  if (mismatchedTypes.size() != 0)
  {
    if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
      ": pokemon inputStream - " << mismatchedTypes.size() << " Orphaned pokemon-types!\n";
    if (verbose >= 6)
    {
      for (size_t indexOrphan = 0; indexOrphan < mismatchedTypes.size(); indexOrphan++)
      {
        std::cout << "\tOrphaned type \"" << mismatchedTypes.at(indexOrphan) << "\"\n";
      }
    }
  }
  if (mismatchedAbilities.size() != 0)
  {
    if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
      ": pokemon inputStream  - " << mismatchedAbilities.size() << " Orphaned pokemon-abilities!\n";
    if (verbose >= 6)
    {
      for (size_t indexOrphan = 0; indexOrphan < mismatchedAbilities.size(); indexOrphan++)
      {
        std::cout << "\tOrphaned ability \"" << mismatchedAbilities.at(indexOrphan) << "\"\n";
      }
    }
  }

  return true; // import success
} // endof import pokemon


bool Pokemons::loadMovelistFromFile(const std::string& path, const Moves& moves) {
  /*
   * Header data:
   * PKAIZ <SIZE> #fluff\n
   * #fluff line\n
   * <String Pokemon>\t<String Movename>
   */

  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAIZ", inputBuffer) != true)
    {
      return false;
    }

    //tokenize by line endings
    lines = tokenize(inputBuffer, "\n\r");
  }

  size_t iLine = 0;
  bool result = loadMovelistFromFile_lines(moves, lines, iLine);
  assert(iLine == lines.size());
  return result && (iLine == lines.size());

}//endof import movelist


bool Pokemons::loadMovelistFromFile_lines(
    const Moves& moves,
    const std::vector<std::string>& lines,
    size_t& iLine) {
  static const std::string header = "PKAIZ";
  /*
   * Header data:
   * PKAIZ <SIZE> #fluff\n
   * #fluff line\n
   * <String Pokemon>\t<String Movename>
   */

  // are the enough lines in the input stream for at least the header:
  if ((lines.size() - iLine) < 2U)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": unexpected end of input stream at line " << iLine << "!\n";
    return false;
  }

  // compare header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": movelist inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) <<
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // guess size of array:
  size_t _numElements;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

    if (!INI::setArgAndPrintError("movelist numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

    if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXMOVELIST)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXMOVELIST)) { return false; }

  // check if pokemon has been initialized yet
  if (empty() || moves.empty())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": Pokemon and moves must be initialized before adding movelists to them!\n";
    return false;
  }

  std::vector<std::string> mismatchedPokemon;
  std::vector<std::string> mismatchedMoves;

  Pokemons::iterator cPokemon = begin();
  Moves::const_iterator cMove = moves.begin();

  for (size_t iPair = 0; iPair < lines.size() - 2; ++iPair, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    if (tokens.size() != 2)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": movelist inputStream has malformed line #" << (iLine) <<
        " with " << tokens.size() << " values!\n";
      return false;
    }

    //find pokemon referred to by this pair
    //TODO: make sure this is actually faster than binary search
    //WARNING: THIS CODE ASSUMES: NO DUPLICATES, ALPHABETICAL ORDER OF POKEMON
    while(cPokemon->getName().compare(tokens.at(0)) != 0)
    {
      cPokemon++;
      if (cPokemon != end())
      {
        cMove = moves.begin(); // reset moves
      }
      else // unable to find target pokemon in move pair:
      {
        // add orphan:
        orphanAddToVector(mismatchedPokemon, tokens.at(0));

        // reset pokemon and moves:
        cPokemon = begin();
        cMove = moves.begin();

        goto outerWhile;
      }
    }

    //find move referred to by this pair
    //TODO: make sure this is actually faster than binary search
    //WARNING: THIS CODE ASSUMES: NO DUPLICATES, ALPHABETICAL ORDER OF MOVES
    while(cMove->getName().compare(tokens.at(1)) != 0)
    {
      cMove++;
      if (cMove == moves.end())
      {
        // add orphan:
        orphanAddToVector(mismatchedMoves, tokens.at(1));

        // reset moves
        cMove = moves.begin();

        goto outerWhile;
      }
    }
    // add this name to the current Pokemon's move list.
    cPokemon->movelist_.push_back(&(*cMove));

  outerWhile:
    std::cerr.flush(); // dummy function
  } //end of per-pair

  // mark orphans with no movesets, sort the movesets of bases which have them:
  for (size_t iPokemon = 0; iPokemon != size(); ++iPokemon)
  {
    PokemonBase& cBase = at(iPokemon);
    if (cBase.movelist_.empty()) { cBase.lostChild_ = true; continue;}
    std::sort(cBase.movelist_.begin(), cBase.movelist_.end());
  }

  //output orphans
  if (mismatchedPokemon.size() != 0)
  {
    if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
      ": movelist inputStream - " << mismatchedPokemon.size() << " Orphaned movelist-pokemon!\n";
    if (verbose >= 6)
    {
      for (size_t indexOrphan = 0; indexOrphan < mismatchedPokemon.size(); indexOrphan++)
      {
        std::cout << "\tOrphaned pokemon \"" << mismatchedPokemon.at(indexOrphan) << "\"\n";
      }
    }
  }
  if (mismatchedMoves.size() != 0)
  {
    if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
      ": movelist inputStream - " << mismatchedMoves.size() << " Orphaned movelist-moves!\n";
    if (verbose >= 6)
    {
      for (size_t indexOrphan = 0; indexOrphan < mismatchedMoves.size(); indexOrphan++)
      {
        std::cout << "\tOrphaned move \"" << mismatchedMoves.at(indexOrphan) << "\"\n";
      }
    }
  }

  return true; // import success
}//endof import movelist
