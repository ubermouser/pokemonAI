#include "../inc/item.h"

#include <iostream>

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"
#include "../inc/type.h"

using namespace INI;
using namespace orphan;


const Item* Item::no_item = NULL;

Item::Item() 
  : Name(), 
  Pluggable(),
  boostedType_(Type::no_type),
  naturalGiftType_(Type::no_type),
  resistedType_(Type::no_type),
  flingPower_(0),
  naturalGiftPower_(0),
  lostChild_(true)
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

  {
    // find special case no item:
    Item::no_item = count("none")?&at("none"):new Item();
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
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)SIZE_MAX)) { return false; }
  reserve(_numElements);

  orphan::OrphanSet orphanTypes;

  for (size_t iItem = 0; iLine < lines.size(); ++iItem, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    Item cItem;
    if (tokens.size() != 8)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": item inputStream has malformed line #" << iLine <<
        " with " << tokens.size() << " values!\n";
      return false;
    }

    cItem.lostChild_ = false;

    //item name
    size_t iToken =  0;
    cItem.setName(lowerCase(tokens.at(iToken)));
    cItem.index_ = iItem;

    // fling power:
    iToken = 1;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.flingPower_ = 0; }
    else
    {
      uint32_t cPower;
      if (!setArg(tokens.at(iToken), cPower)) { incorrectArgs("fling power", iLine, iToken); return false; }
      checkRangeB(cPower, 1U, 255U);
      cItem.flingPower_ = (uint8_t) cPower;
    }

    // boosted type:
    iToken = 2;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.boostedType_ = Type::no_type; }
    else
    {
      const Type* cType = orphanCheck(types, tokens.at(iToken), &orphanTypes);
      if (cType == NULL) { cItem.lostChild_ = true; } //orphan!
      cItem.boostedType_ = cType;
    }

    // IGNORED: pokemon affected

    // natural gift power:
    iToken = 4;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.naturalGiftPower_ = 0; }
    else
    {
      uint32_t cPower;
      if (!setArg(tokens.at(iToken), cPower)) { incorrectArgs("natural gift power", iLine, iToken); return false; }
      checkRangeB(cPower, 1U, 255U);
      cItem.naturalGiftPower_ = (uint8_t) cPower;
    }

    // natural gift type:
    iToken = 5;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.naturalGiftType_ = Type::no_type; }
    else
    {
      cItem.naturalGiftType_ = orphanCheck(types, tokens.at(iToken), &orphanTypes);
      if (cItem.naturalGiftType_ == NULL) { cItem.lostChild_ = true; } //orphan!
    }

    // resisted type:
    iToken = 6;
    if (tokens.at(iToken).compare("---") == 0)
    { cItem.resistedType_ = Type::no_type; }
    else
    {
      cItem.resistedType_ = orphanCheck(types, tokens.at(iToken), &orphanTypes);
      if (cItem.resistedType_ == NULL) { cItem.lostChild_ = true; } //orphan!
    }

    // item script
    iToken = 7;
    if (tokens.at(iToken).compare("---") == 0) {
      cItem.setHasNoPlugins();
    }

    insert({cItem.getName(), cItem});
    byIndex_.insert({cItem.index_, &at(cItem.getName())});
  } //end of per-item

  printOrphans(orphanTypes, "item inputStream", "item-types", "type");
  return true;
} // end of importItem
