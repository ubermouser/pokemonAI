#include "pokemonai/ability.h"

#include <ostream>

#include "pokemonai/init_toolbox.h"
#include "pokemonai/orphan.h"

using namespace INI;
using namespace orphan;

const Ability* Ability::no_ability = NULL;


bool Abilities::initialize(const std::string& path) {
  if (path.empty())
  {
    SPDLOG_CRITICAL("An ability list has not been defined!");
    return false;
  }
  SPDLOG_WARN("Loading Pokemon ability library at {}...", path);
  if (!loadFromFile(path))
  {
    SPDLOG_CRITICAL(
        "inputAbilities failed to populate a list of pokemon abilities.");
    return false;
  }

  // find special case no ability:
  if (Ability::no_ability == nullptr) {
    Ability::no_ability = count("none") ? &at("none") : new Ability();
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
    SPDLOG_CRITICAL("Unexpected end of input stream at line {}!", iLine);
    return false;
  }

  // compare header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    SPDLOG_CRITICAL(
        "Ability inputStream has header of type \"{}\" (needs to be "
        "\"{}\") and is incompatible with this program!",
        lines.at(iLine).substr(0, header.size()),
        header);

    return false;
  }

  // guess size of array:
  size_t _numElements;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

    if (!INI::setArgAndPrintError("abilities numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)SIZE_MAX)) { return false; }
  reserve(_numElements);
  size_t num_loaded = 0;

  for (size_t iAbility = 0; iLine < lines.size(); ++iAbility, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    Ability cAbility;
    if (tokens.size() != 2)
    {
      SPDLOG_CRITICAL(
          "Ability inputStream has malformed line #{} with {} values!",
          iLine,
          tokens.size());
      return false;
    }

    //ability name
    cAbility.setName(lowerCase(tokens.at(0)));

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

    cAbility.index_ = iAbility;
    insert(cAbility);
    num_loaded++;
  } //end of per-ability

  SPDLOG_INFO("Loaded {} abilities!", num_loaded);

  return true; // import success
}
