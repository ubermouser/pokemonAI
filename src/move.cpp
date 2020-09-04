#include "../inc/move.h"

#include <algorithm>

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

#include "../inc/move_nonvolatile.h"
#include "../inc/type.h"
#include "../inc/pokedex.h"

using namespace INI;
using namespace orphan;

const Move* Move::move_struggle = NULL;
const Move* Move::move_none = NULL;

Move::Move(
    const std::string& name,
    const Type* type,
    int32_t primaryAccuracy,
    uint32_t power,
    uint32_t PP,
    uint32_t damageType,
    int32_t target,
    int32_t priority,
    int32_t secondaryAccuracy,
    const BuffModArray& selfBuff,
    const BuffModArray& targetDebuff,
    uint32_t targetAilment,
    uint32_t targetVolatileAilment,
    bool hasPlugins,
    const std::string& description) :
    Name(name),
    Pluggable(),
    type_(type),
    primaryAccuracy_(primaryAccuracy),
    power_(power),
    PP_(PP),
    damageType_(damageType),
    target_(target),
    priority_(priority),
    secondaryAccuracy_(secondaryAccuracy),
    selfBuff_(selfBuff),
    targetDebuff_(targetDebuff),
    targetAilment_(targetAilment),
    targetVolatileAilment_(targetVolatileAilment),
    description_(description),
    lostChild(false)
{
  if (!hasPlugins) { setHasNoPlugins(); }
  checkRangeB(primaryAccuracy, -1, 100);
  checkRangeB(power, 0, 254);
  checkRangeB(PP, 5, 40);
  checkRangeB(damageType, 0, 3);
  checkRangeB(target, -1, 8);
  checkRangeB(priority, -5, 5);
  checkRangeB(*std::max_element(begin(selfBuff), end(selfBuff)), -12, 12);
  checkRangeB(*std::min_element(begin(selfBuff), end(selfBuff)), -12, 12);
  checkRangeB(*std::max_element(begin(targetDebuff), end(targetDebuff)), -12, 12);
  checkRangeB(*std::min_element(begin(targetDebuff), end(targetDebuff)), -12, 12);
  checkRangeB(targetAilment, AIL_NV_NONE, AIL_NV_MAX);
  if (type == NULL) { type_ = Type::no_type; lostChild = true; }
}


const Type& Move::getType() const
{
  assert(type_ != NULL);
  return *type_;
}


bool Moves::initialize(const std::string& path, const Types& types) {
  if (path.empty())
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": A move list has not been defined!\n";
    return false;
  }
  if (verbose >= 1) std::cout << " Loading Pokemon move library...\n";
  if (!loadFromFile(path, types))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": inputMoves failed to populate a list of moves.\n";
    return false;
  }

  {
    // find special cases struggle and hurt confusion, set the pointers to them
    Move::move_none = count("none")?&at("none"):new Move();
    Move::move_struggle = &at("struggle");
    MoveNonVolatile::mNV_struggle = new MoveNonVolatile(*Move::move_struggle);
  }
  
  return true;
}


bool Moves::loadFromFile(const std::string& path, const Types& types) {
  /*
   * Header data:
   * PKAIM <SIZE> #fluff\n
   * #fluff line\n
   * <String NAME>\t<int TYPE>\t<int PP>\t<int P.Accuracy>\t<int Power>\t<int Damage>\t<int Target>\t<int Priority>\t ... etc
   */

  std::vector<std::string> lines;
  {
    std::string inputBuffer;

    if (loadFileToString(path, "PKAIM", inputBuffer) != true)
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
} // end of inputMoves


template<class IntegerType>
bool intFromToken(
    const std::vector<std::string> tokens,
    size_t iLine,
    size_t iToken,
    const std::string& argName,
    IntegerType& result,
    IntegerType default_value=IntegerType(0)) {
  const auto& token = tokens.at(iToken);
  if (token == "---" || token == "Var") {
    result = default_value;
  } else if (!setArg(tokens.at(iToken), result)) {
    incorrectArgs(argName, iLine, iToken);
    return false;
  }
  return true;
}


template<size_t length, class IntegerType, class DefaultType>
bool integerArrayFromTokens(
    const std::vector<std::string> tokens,
    size_t iLine,
    size_t iToken,
    const std::string& argName,
    std::array<IntegerType, length>& result,
    DefaultType default_value=0) {
  for (size_t iElem = 0; iElem < length; iToken++, iElem++) {
    if (!intFromToken(tokens, iLine, iToken, argName, result[iElem], IntegerType(default_value))) {
      return false;
    }
  }

  return true;
}


bool ailmentFromToken(
    const std::vector<std::string> tokens,
    size_t iLine,
    size_t iStartToken,
    uint32_t& result) {
  uint32_t ailment;
  result = AIL_NV_NONE;

  // ailment,target,burn
  size_t iToken = iStartToken + 0;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = AIL_NV_BURN; goto endAilment; }
  }
  else { incorrectArgs("AIL_NV_BURN", iLine, iToken); return false; }

  // ailment,target,Freeze
  iToken = iStartToken + 1;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = AIL_NV_FREEZE; goto endAilment; }
  }
  else { incorrectArgs("AIL_NV_FREEZE", iLine, iToken); return false; }

  // ailment,target,paralysis
  iToken = iStartToken + 2;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = AIL_NV_PARALYSIS; goto endAilment; }
  }
  else { incorrectArgs("AIL_NV_PARALYSIS", iLine, iToken); return false; }

  // ailment,target,poison
  iToken = iStartToken + 3;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)2);
    if (ailment == 1) { result = AIL_NV_POISON; goto endAilment; }
    else if(ailment == 2) { result = AIL_NV_POISON_TOXIC; goto endAilment; }
  }
  else { incorrectArgs("AIL_NV_POISON/AIL_NV_POISON_TOXIC", iLine, iToken); return false; }

  // ailment,target,sleep
  iToken = iStartToken + 4;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = AIL_NV_SLEEP; goto endAilment; }
  }
  else { incorrectArgs("AIL_NV_SLEEP", iLine, iToken); return false; }

  result = AIL_V_NONE;

  endAilment:
  return true;
}


bool volatileAilmentFromToken(
    const std::vector<std::string> tokens,
    size_t iLine,
    size_t iStartToken,
    uint32_t& result) {
  uint32_t ailment;
  result = AIL_V_NONE;
  // volatileAilment,target,confusion
  size_t iToken = iStartToken + 0;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = AIL_V_CONFUSED; goto endVolatile; }
  }
  else { incorrectArgs("AIL_V_CONFUSED", iLine, iToken); return false; }

  // volatileAilment,target,flinch
  iToken = iStartToken + 1;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = AIL_V_FLINCH; goto endVolatile; }
  }
  else { incorrectArgs("AIL_V_FLINCH", iLine, iToken); return false; }

  // volatileAilment,target,identify
  iToken = iStartToken + 2;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = 110; goto endVolatile; }
  }
  else { incorrectArgs("AIL_V_IDENTIFY", iLine, iToken); return false; }

  // volatileAilment,target,infatuation
  iToken = iStartToken + 3;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = AIL_V_INFATUATED; goto endVolatile; }
  }
  else { incorrectArgs("AIL_V_INFATUATED", iLine, iToken); return false; }

  // volatileAilment,target,lock on
  iToken = iStartToken + 4;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = 111; goto endVolatile; }
  }
  else { incorrectArgs("AIL_V_LOCKON", iLine, iToken); return false; }

  // volatileAilment,target,nightmare
  iToken = iStartToken + 5;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = 112; goto endVolatile; }
  }
  else { incorrectArgs("AIL_V_NIGHTMARE", iLine, iToken); return false; }

  // volatileAilment,target,partial trap
  iToken = iStartToken + 6;
  if (setArg(tokens.at(iToken), ailment))
  {
    checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
    if (ailment == 1) { result = 113; goto endVolatile; }
  }
  else { incorrectArgs("AIL_V_PARTIALTRAP", iLine, iToken); return false; }

  endVolatile:
  return true;
}


bool Moves::loadFromFile_lines(
    const Types& types, const std::vector<std::string>& lines, size_t& iLine) {
  static const std::string header = "PKAIM";
  /*
   * Header data:
   * PKAIM\t<SIZE>\t#fluff\n
   * #fluff line\n
   * <String NAME>\t<int TYPE>\t<int PP>\t<int P.Accuracy>\t<int Power>\t<int Damage>\t<int Target>\t<int Priority>\t ... etc
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
      ": move inputStream has header of type \"" << lines.at(iLine).substr(0, header.size()) <<
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  // guess size of move array:
  size_t _numElements;
  {
    std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    if (!INI::checkRangeB(tokens.size(), (size_t)2, tokens.max_size())) { return false; }

    if (!INI::setArgAndPrintError("moves numElements", tokens.at(1), _numElements, iLine, 1)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)SIZE_MAX)) { return false; }
  reserve(_numElements);

  orphan::OrphanSet orphanTypes;

  for (size_t iMove = 0; iLine != lines.size(); ++iMove, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    if (tokens.size() != 41)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": move inputStream has malformed line #" << iLine <<
        " with " << tokens.size() << " values!\n";
      return false;
    }
    std::string name, description;
    bool hasPlugins;
    const Type* type;
    uint32_t PP, power, damageType, targetAilment, targetVolatileAilment;
    int32_t target, priority, primaryAccuracy, secondaryAccuracy;
    Move::BuffModArray selfBuff, targetDebuff;

    name = lowerCase(tokens.at(0)); // move name
    type = orphanCheck(types, tokens.at(1), &orphanTypes); // type
    if (!intFromToken(tokens, iLine, 2, "PP", PP)) { return false; }
    if (!intFromToken(tokens, iLine, 3, "primaryAccuracy", primaryAccuracy, -1)) { return false; }
    if (!intFromToken(tokens, iLine, 4, "Power", power)) { return false; }
    if (!intFromToken(tokens, iLine, 5, "DamageType", damageType)) { return false; }
    if (!intFromToken(tokens, iLine, 6, "Target", target, -1)) { return false; }
    if (!intFromToken(tokens, iLine, 7, "Priority", priority, 0)) { return false; }
    if (!integerArrayFromTokens(tokens, iLine, 8, "buff", selfBuff, 0)) { return false; }
    if (!intFromToken(tokens, iLine, 17, "secondaryAccuracy", secondaryAccuracy, -1)) { return false; }
    if (!integerArrayFromTokens(tokens, iLine, 18, "debuff", targetDebuff, 0)) { return false; }
    if (!ailmentFromToken(tokens, iLine, 27, targetAilment)) { return false; }
    if (!volatileAilmentFromToken(tokens, iLine, 32, targetVolatileAilment)) { return false; }
    hasPlugins = tokens.at(39) != "---";
    description = tokens.at(40);

    Move move(
        name,
        type,
        primaryAccuracy,
        power,
        PP,
        damageType,
        target,
        priority,
        secondaryAccuracy,
        selfBuff,
        targetDebuff,
        targetAilment,
        targetVolatileAilment,
        hasPlugins,
        description);
    insert({name, move});
    if (verbose >= 6) std::cout << "\tLoaded move " << iMove << "-\"" << move.getName() << "\"\n";
  } //end of per-move

  if (orphanTypes.size() != 0)
  {
    if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
      ": move inputStream - " << orphanTypes.size() << " Orphaned move-types!\n";
    if (verbose >= 6)
    {
      for (auto orphan : orphanTypes) {
        std::cout << "\tOrphaned type \"" << orphan << "\"\n";
      }
    }
  }

  return true;
} // end of inputMoves
