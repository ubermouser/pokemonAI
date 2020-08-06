#include "../inc/type.h"

#include "../inc/pokedex.h"

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

using namespace INI;
using namespace orphan;

const Type* Type::no_type = NULL;


Type::Type() 
  : modTable_()
{
}


Type::Type(const Type& source) 
  : modTable_(source.modTable_)
{
}


fpType Type::getModifier(const Type& other) const
{
  size_t iOType = &other - &pkdex->getTypes().front();

  return (fpType)modTable_[iOType] / (fpType) FPMULTIPLIER;
}


bool Types::initialize(const std::string& path) {
  //TYPE library
  if (path.empty())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": An item list has not been defined!\n";
    return false;
  }
  if (verbose >= 1) std::cout << " Loading Pokemon type library...\n";
  if (!loadFromFile(path))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": InputTypes failed to populate an array of types.\n";
    return false;
  }
  //std::sort(types.begin(), types.end());
  {
    // find type special case
    Type::no_type = orphan::orphanCheck_ptr(*this, NULL, "none");
  }
  
  return true;
}


bool Types::loadFromFile(const std::string& path) {
  /*
   * Header data:
   * PKAIT <SIZE> #fluff\n
   * #fluff line\n
   * <String NAME>\t<float 1>\t<float 2>\t<float ... SIZE>
   */

  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAIT", inputBuffer) != true)
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
} // endof import types


bool Types::loadFromFile_lines(const std::vector<std::string>& lines, size_t& iLine) {
  static const std::string header = "PKAIT";
  /*
   * Header data:
   * PKAIT <SIZE> #fluff\n
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
      ": types inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) <<
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // guess size of array:
  size_t _numElements;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

    if (!INI::setArgAndPrintError("types numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

    if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXTYPES)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXTYPES)) { return false; }
  resize(_numElements);

  for (size_t iType = 0; iType < size(); ++iType, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    class Type& cType = at(iType);
    if (tokens.size() != size()+1)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": types inputStream has malformed line #" << iLine <<
        " with " << tokens.size() << " values!\n";
      return false;
    }

    //allocate dynamic modtable
    cType.modTable_.resize(size(), 0);

    //type name
    cType.setName(tokens.at(0));

    //dynamic modtable
    for (size_t indexInnerType = 0; indexInnerType < size(); indexInnerType++)
    {
      double cTypeVal;
      if (!setArg(tokens.at(indexInnerType+1), cTypeVal)) { incorrectArgs("cTypeVal", iLine, indexInnerType+1); return false; }
      checkRangeB(cTypeVal, 0.0, 2.0);
      cType.modTable_[indexInnerType] = cTypeVal * FPMULTIPLIER;
    }

  } //end of per-type

  return true; // import success
} // endof import types
