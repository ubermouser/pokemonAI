#include "../inc/item.h"

#include <ostream>
#include <algorithm>

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"
#include "../inc/type.h"

using namespace INI;
using namespace orphan;


const Item* Item::no_item = NULL;

Item::Item() 
  : Name(), 
  Pluggable(),
  boostedType(Type::no_type),
  naturalGift_type(Type::no_type),
  resistedType(Type::no_type),
  flingPower(0),
  naturalGift_power(0),
  lostChild(true)
{
};


Item::Item(const Item& source) 
  : Name(source), 
  Pluggable(source),
  boostedType(source.boostedType),
  naturalGift_type(source.naturalGift_type),
  resistedType(source.resistedType),
  flingPower(source.flingPower),
  naturalGift_power(source.naturalGift_power),
  lostChild(source.lostChild)
{
};


bool Items::initialize(const std::string& path, const Types& types) {
  if (path.empty())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": An item list has not been defined!\n";
    return false;
  }
  if (verbose >= 1) std::cout << " Loading Pokemon item library...\n";
  if (!loadFromFile(path, types))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": inputItems failed to populate a list of pokemon items.\n";
    return false;
  }
  std::sort(begin(), end());

  {
    // find special case no item:
    Item::no_item = orphan::orphanCheck_ptr(*this, NULL, "none");
  }
  
  return true;
}


bool Items::loadFromFile(const std::string& path, const Types& types) {
  /*
   * Header data:
   * PKAII <SIZE> #fluff\n
   * #fluff line\n
   * <String NAME>\t<String SCRIPT>
   */

  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAII", inputBuffer) != true)
    {
      return false;
    }

    //tokenize by line endings
    lines = tokenize(inputBuffer, "\n\r");
  }

  size_t iLine = 0;
  bool result = loadFromFile_lines(types, lines, iLine);
  assert(iLine == lines.size());
  return result && (iLine == lines.size());
} // end of importItem


bool Items::loadFromFile_lines(
    const Types& types, const std::vector<std::string>& lines, size_t& iLine) {
  static const std::string header = "PKAII";
  /*
   * Header data:
   * PKAII <SIZE> #fluff\n
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
      ": item inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) <<
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // guess size of item array:
  size_t _numElements;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

    if (!INI::setArgAndPrintError("items numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }

    if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXITEMS)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXITEMS)) { return false; }
  resize(_numElements);

  std::vector<std::string> mismatchedTypes;

  for (size_t iItem = 0; iItem < size(); ++iItem, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    class Item& cItem = at(iItem);
    if (tokens.size() != 8)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": item inputStream has malformed line #" << iLine <<
        " with " << tokens.size() << " values!\n";
      return false;
    }

    cItem.lostChild = false;

    //item name
    size_t iToken =  0;
    cItem.setName(tokens.at(iToken));

    // fling power:
    iToken = 1;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.flingPower = 0; }
    else
    {
      uint32_t cPower;
      if (!setArg(tokens.at(iToken), cPower)) { incorrectArgs("fling power", iLine, iToken); return false; }
      checkRangeB(cPower, 1U, 255U);
      cItem.flingPower = (uint8_t) cPower;
    }

    // boosted type:
    iToken = 2;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.boostedType = Type::no_type; }
    else
    {
      const Type* cType = orphanCheck_ptr(types, &mismatchedTypes, tokens.at(iToken));
      if (cType == NULL) { cItem.lostChild = true; } //orphan!
      cItem.boostedType = cType;
    }

    // IGNORED: pokemon affected

    // natural gift power:
    iToken = 4;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.naturalGift_power = 0; }
    else
    {
      uint32_t cPower;
      if (!setArg(tokens.at(iToken), cPower)) { incorrectArgs("natural gift power", iLine, iToken); return false; }
      checkRangeB(cPower, 1U, 255U);
      cItem.naturalGift_power = (uint8_t) cPower;
    }

    // natural gift type:
    iToken = 5;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.naturalGift_type = Type::no_type; }
    else
    {
      const Type* cType = orphanCheck_ptr(types, &mismatchedTypes, tokens.at(iToken));
      if (cType == NULL) { cItem.lostChild = true; } //orphan!
      cItem.naturalGift_type = cType;
    }

    // resisted type:
    iToken = 6;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.resistedType = Type::no_type; }
    else
    {
      const Type* cType = orphanCheck_ptr(types, &mismatchedTypes, tokens.at(iToken));
      if (cType == NULL) { cItem.lostChild = true; } //orphan!
      cItem.resistedType = cType;
    }

    // item script
    iToken = 7;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.setHasNoPlugins(); }
    /*else
    {
      size_t tokenLength = tokens.at(iToken).size();
      size_t offset = 0;
      //check for quotations, and if they exist remove them
      if (tokens.at(1)[0] == '"' && tokens.at(iToken)[tokenLength-1] == '"')
      {
        offset = 1;
      }

      cItem.script = std::string(tokens.at(iToken).substr(offset, tokenLength - offset));
    }*/
  } //end of per-item

  if (mismatchedTypes.size() != 0)
  {
    if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
      ": item inputStream - " << mismatchedTypes.size() << " Orphaned item-types!\n";
    if (verbose >= 6)
    {
      for (size_t indexOrphan = 0; indexOrphan < mismatchedTypes.size(); indexOrphan++)
      {
        std::cout << "\tOrphaned type \"" << mismatchedTypes.at(indexOrphan) << "\"\n";
      }
    }
  }

  return true;
} // end of importItem
