#include "../inc/ability.h"

#include <algorithm>
#include <ostream>

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

using namespace INI;
using namespace orphan;

const Ability* Ability::no_ability = NULL;


bool Abilities::initialize(const std::string& path) {
  if (path.empty())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": An ability list has not been defined!\n";
    return false;
  }
  if (verbose >= 1) std::cout << " Loading Pokemon ability library...\n";
  if (!loadFromFile(path))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": inputAbilities failed to populate a list of pokemon abilities.\n";
    return false;
  }
  std::sort(begin(), end());

  {
    // find special case no ability:
    Ability::no_ability = orphan::orphanCheck_ptr(*this, NULL, "none");
  }
  
  return true;
}


bool Abilities::loadFromFile(const std::string& path)
{
  /*
   * Header data:
   * PKAIA <SIZE> #fluff\n
   * #fluff line\n
   * <String NAME>\t<String SCRIPT>
   */

  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAIA", inputBuffer) != true)
    {
      return false;
    }

    //tokenize by line endings
    lines = tokenize(inputBuffer, "\n\r");
  }

  size_t iLine = 0;
  bool result = loadFromFile_lines(lines, iLine);
  assert(iLine == lines.size());
  return result && (iLine == lines.size());
}


bool Abilities::loadFromFile_lines(const std::vector<std::string>& lines, size_t& iLine) {
  static const std::string header = "PKAIA";
  /*
   * Header data:
   * PKAIA <SIZE> #fluff\n
   * #fluff line\n
   * <String NAME>\t<String SCRIPT>
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
      ": ability inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) <<
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // guess size of array:
  size_t _numElements;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

    if (!INI::setArgAndPrintError("abilities numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

    if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXABILITIES)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXABILITIES)) { return false; }
  resize(_numElements);

  for (size_t iAbility = 0; iAbility < size(); ++iAbility, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    class Ability& cAbility = at(iAbility);
    if (tokens.size() != 2)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": ability inputStream has malformed line #" << iLine <<
        " with " << tokens.size() << " values!\n";
      return false;
    }

    //ability name
    cAbility.setName(tokens.at(0));

    //ability script
    if (tokens.at(1).compare("---") == 0)
    { cAbility.script_.clear(); }
    else
    {
      size_t tokenLength = tokens.at(1).size();
      size_t offset = 0;
      //check for quotations, and if they exist remove them
      if (tokens.at(1)[0] == '"' && tokens.at(1)[tokenLength-1] == '"')
      {
        offset = 1;
      }

      cAbility.script_ = std::string(tokens.at(1).substr(offset, tokenLength - offset));
    }

  } //end of per-ability

  return true; // import success
}
