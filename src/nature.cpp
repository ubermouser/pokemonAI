#include "pokemonai/nature.h"

#include "pokemonai/init_toolbox.h"
#include "pokemonai/orphan.h"

using namespace INI;
using namespace orphan;

const Nature* Nature::no_nature = NULL;


bool Natures::initialize(const std::string& path) {
  if (path.empty())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": A nature list has not been defined!\n";
    return false;
  }
  if (verbose >= 1) std::cout << " Loading Pokemon nature library...\n";
  if (!loadFromFile(path))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": inputNatures failed to populate a list of natures.\n";
    return false;
  }

  {
    // find special case no nature:
    Nature::no_nature = count("none")?&at("none"):new Nature();
  }
  
  return true;
}


bool Natures::loadFromFile(const std::string& path)
{
  /*
   * Header data:
   * PKAIN <SIZE> #fluff\n
   * #fluff line\n
   * <String NAME>\t<float 1>\t<float 2>\t<float ... SIZE>
   */

  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAIN", inputBuffer) != true)
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
} // endof import natures


bool Natures::loadFromFile_lines(const std::vector<std::string>& lines, size_t& iLine)
{
  static const std::string header = "PKAIN";
  /*
   * Header data:
   * PKAIN <SIZE> #fluff\n
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
      ": natures inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) <<
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // guess size of array:
  size_t _numElements;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

    if (!INI::setArgAndPrintError("natures numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)SIZE_MAX)) { return false; }
  reserve(_numElements);

  for (size_t iNature = 0; iLine < lines.size(); ++iNature, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    class Nature cNature;
    if (tokens.size() != 6)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": natures inputStream has malformed line #" << iLine <<
        " with " << tokens.size() << " values!\n";
      return false;
    }

    //nature name
    cNature.setName(lowerCase(tokens.at(0)));

    //modtable
    for (size_t indexMod = 0; indexMod < 5; indexMod++)
    {
      double cNatureVal;
      if (!setArg(tokens.at(indexMod+1), cNatureVal)) { incorrectArgs("cNatureVal", iLine, indexMod+1); return false; }
      checkRangeB(cNatureVal, 0.9, 1.1);
      cNature.modTable_[indexMod] = cNatureVal * FPMULTIPLIER;
    }

    insert({cNature.getName(), cNature});
  } //end of per-nature

  return true; // import success
} // endof import natures
