#include "pokemonai/pokemon_base.h"

#include "pokemonai/init_toolbox.h"
#include "pokemonai/orphan.h"

#include "pokemonai/ability.h"
#include "pokemonai/type.h"
#include "pokemonai/move.h"

using namespace INI;
using namespace orphan;

const PokemonBase* PokemonBase::no_base = NULL;

PokemonBase::PokemonBase(
    const std::string& name,
    const TypeArray& types,
    uint16_t weight,
    const StatsArray& stats,
    const AbilitySet& abilities,
    const MoveSet& moves) :
    Name(name),
    types_(types),
    weight_(weight),
    baseStats_(stats),
    abilities_(abilities),
    moves_(moves) {
  checkRangeB(weight, (fpType)0.1, (fpType)1000.0);
}


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

  {
    // find special case no base:
    PokemonBase::no_base = count("none")?&at("none"):new PokemonBase();
  }

  //MOVELIST library
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
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)SIZE_MAX)) { return false; }
  reserve(_numElements);

  OrphanSet orphanedTypes;
  OrphanSet orphanedAbilities;

  for (size_t iPokemon = 0; iLine < lines.size(); ++iPokemon, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    PokemonBase cPokemon;
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
    cPokemon.setName(lowerCase(tokens.at(iToken)));

    //pokemon primary type
    iToken = 1;
    {
      const Type* type = orphanCheck(types, tokens.at(iToken), &orphanedTypes);
      if (type == NULL) { cPokemon.lostChild_ = true; } //orphan!
      cPokemon.types_[0] = type;
    }

    //pokemon secondary type
    iToken = 2;
    {
      const Type* type = orphanCheck(types, tokens.at(iToken), &orphanedTypes);
      if (type == NULL) { cPokemon.lostChild_ = true; } //orphan!
      cPokemon.types_[1] = type; // notype = "None"
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
      const Ability* cAbility = orphanCheck(abilities, tokens.at(iToken), &orphanedAbilities);
      if (cAbility == NULL) { cPokemon.lostChild_ = true; } //orphan!
      else { cPokemon.abilities_.insert(cAbility); }
    }

    //second ability choice
    iToken = 11;
    if (tokens.at(iToken) != "---")
    {
      //strncpy(currentPokemon->secondaryAbilityName,tokens.at(11),20);
      const Ability* cAbility = orphanCheck(abilities, tokens.at(iToken), &orphanedAbilities);
      if (cAbility == NULL) { cPokemon.lostChild_ = true; } //orphan!
      else { cPokemon.abilities_.insert(cAbility); }
    }

    insert({cPokemon.getName(), cPokemon});
  } //end of per-pokemon

  //output orphans
  printOrphans(orphanedTypes, "pokemon inputStream", "pokemon-types", "type");
  printOrphans(orphanedTypes, "pokemon inputStream", "pokemon-abilities", "ability");
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

  OrphanSet orphanedPokemon;
  OrphanSet orphanedMoves;

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

    PokemonBase* cPokemon = orphanCheck(*this, tokens.at(0), &orphanedPokemon);
    const Move* cMove = orphanCheck(moves, tokens.at(1), &orphanedMoves);

    // ignore orphan pairs
    if (cPokemon == NULL || cMove == NULL) { continue; }
    // add this name to the current Pokemon's move list.
    cPokemon->moves_.insert(cMove);
  } //end of per-pair

  // mark orphans with no movesets:
  for (auto& cBase : *this) {
    if (cBase.second.moves_.empty()) { cBase.second.lostChild_ = true; continue;}
  }

  //output orphans
  printOrphans(orphanedPokemon, "movelist inputStream", "movelist-pokemon", "pokemon");
  printOrphans(orphanedPokemon, "movelist inputStream", "movelist-moves", "move");
  return true; // import success
}//endof import movelist
