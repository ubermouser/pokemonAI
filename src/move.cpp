#include "../inc/move.h"

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

#include "../inc/move_nonvolatile.h"
#include "../inc/type.h"
#include "../inc/pokedex.h"

using namespace INI;
using namespace orphan;

const Move* Move::move_struggle = NULL;
const Move* Move::move_none = NULL;

void Move::init(const Move& source)
{
  cType = source.cType;
  
  primaryAccuracy = source.primaryAccuracy;
  secondaryAccuracy = source.secondaryAccuracy;
  
  power = source.power;
  
  PP = source.PP;
  
  damageType = source.damageType;
  
  target = source.target;
  
  priority = source.priority;
  
  // buffs
  for (unsigned int indexBuff = 0; indexBuff < 9; indexBuff++)
  {
    selfBuff[indexBuff] = source.selfBuff[indexBuff];
    targetDebuff[indexBuff] = source.targetDebuff[indexBuff];
  }
  
  targetAilment = source.targetAilment;
  targetVolatileAilment = source.targetVolatileAilment;
  
  description = source.description;
  
  lostChild = source.lostChild;
}


Move::Move()
  : Name(),
  Pluggable(),
  description()
{
  cType = NULL;
  
  primaryAccuracy = UINT8_MAX;
  secondaryAccuracy = -1;
  
  power = 0;
  
  PP = 0;
  
  damageType = 0;
  
  target = -1;
  
  priority = 0;
  
  // buffs
  selfBuff.fill(0);
  targetDebuff.fill(0);
  
  targetAilment = AIL_NV_NONE;
  targetVolatileAilment = AIL_V_NONE;
  
  lostChild = true;
}


Move::Move(const Move& source)
  : Name(source),
  Pluggable(source)
{
  init(source);
}


Move& Move::operator=(const Move& source)
{
  // identity theorem - simply return what we have now if equal address
  if (this == &source) { return *this; } 
  
  Name::operator=(source);
  Pluggable::operator=(source);
  
  init(source);
  
  return *this;
}


Move::~Move()
{
  description.clear();
}


const Type& Move::getType() const
{
  assert(cType != NULL);
  return *cType;
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
  std::sort(begin(), end());

  {
    Move::move_none = orphan::orphanCheck_ptr(*this, NULL, "none");

    // find special cases struggle and hurt confusion, set the pointers to them
    Move::move_struggle = orphan::orphanCheck_ptr(*this, NULL, "struggle");
    if (Move::move_struggle != NULL) { MoveNonVolatile::mNV_struggle = new MoveNonVolatile(*Move::move_struggle); }
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

    if (!INI::checkRangeB(_numElements, (size_t)0, (size_t)MAXMOVES)) { return false; }
  }

  // ignore fluff line
  iLine+=2;

  // make sure number of lines in the file is correct for the number of moves we were given:
  if (!INI::checkRangeB(lines.size() - iLine, (size_t)_numElements, (size_t)MAXMOVES)) { return false; }
  resize(_numElements);

  std::vector<std::string> mismatchedTypes;

  for (size_t iMove = 0; iMove < size(); ++iMove, ++iLine)
  {
    std::vector<std::string> tokens = tokenize(lines.at(iLine), "\t");
    Move& cMove = at(iMove);
    if (tokens.size() != 41)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": move inputStream has malformed line #" << iLine <<
        " with " << tokens.size() << " values!\n";
      return false;
    }
    cMove.lostChild = false;

    // move name
    size_t iToken = 0;
    cMove.setName(tokens.at(iToken));


    // move type
    iToken = 1;
    {
      size_t iType = orphanCheck(types, &mismatchedTypes, tokens.at(iToken));
      if (iType == SIZE_MAX)
      {
        //incorrectArgs("type", iMove, iToken);
        cMove.cType = NULL;
        cMove.lostChild = true;
        //return false;
      } //orphan!
      else { cMove.cType = &types[iType]; }
    }

    // move base PP
    iToken = 2;
    {
      uint32_t currentPP;
      if (!setArg(tokens.at(iToken), currentPP)) { incorrectArgs("currentPP", iLine, iToken); return false; }
      checkRangeB(currentPP, (uint32_t)5, (uint32_t)40);
      cMove.PP = currentPP;
    }

    // move primary accuracy
    iToken = 3;
    if (tokens.at(iToken).compare("---") == 0)
    { cMove.primaryAccuracy = UINT8_MAX; }
    else
    {
      uint32_t cPrimaryAccuracy;
      if (!setArg(tokens.at(iToken), cPrimaryAccuracy)) { incorrectArgs("cPrimaryAccuracy", iLine, iToken); return false; }
      checkRangeB(cPrimaryAccuracy, (uint32_t)1, (uint32_t)100);
      cMove.primaryAccuracy = cPrimaryAccuracy;
    }

    // move power
    iToken = 4;
    if (tokens.at(iToken).compare("Var") == 0 || tokens.at(iToken).compare("---") == 0)
    { cMove.power = 0; }
    else
    {
      uint32_t cPower;
      if (!setArg(tokens.at(iToken), cPower)) { incorrectArgs("cPower", iLine, iToken); return false; }
      checkRangeB(cPower, (uint32_t)0, (uint32_t)254);
      cMove.power = cPower;
    }

    // move damage type
    iToken = 5;
    {
      uint32_t cDamageType;
      if (!setArg(tokens.at(iToken), cDamageType)) { incorrectArgs("cDamageType", iLine, iToken); return false; }
      checkRangeB(cDamageType, (uint32_t)0, (uint32_t)3);
      cMove.damageType = cDamageType;
    }

    // move target
    iToken = 6;
    if (tokens.at(iToken).compare("Var") == 0 || tokens.at(iToken).compare("---") == 0)
    { cMove.target = -1; }
    else
    {
      uint32_t cTarget;
      if (!setArg(tokens.at(iToken), cTarget)) { incorrectArgs("cTarget", iLine, iToken); return false; }
      checkRangeB(cTarget, (uint32_t)0, (uint32_t)7);
      cMove.target = cTarget;
    }

    // move priority
    iToken = 7;
    {
      int32_t cPriority;
      if (!setArg(tokens.at(iToken), cPriority)) { incorrectArgs("cPriority", iLine, iToken); return false; }
      checkRangeB(cPriority, (int32_t)-5, (int32_t)5);
      cMove.priority = cPriority;
    }

    // buffs
    iToken = 8;
    for (unsigned int iBuff = 0; iBuff < 9; iBuff++)
    {
      // buff, self
      int32_t tempBuff;
      if (!setArg(tokens.at(iToken + iBuff), tempBuff)) { incorrectArgs("buff", iLine, iToken + iBuff); return false; }
      checkRangeB(tempBuff, (int32_t)-12, (int32_t)12);
      cMove.selfBuff[iBuff] = tempBuff;
    }

    // move secondary accuracy
    iToken = 17;
    if (tokens.at(iToken).compare("---") == 0)
    { cMove.secondaryAccuracy = -1; }
    else
    {
      uint32_t cSecondaryAccuracy;
      if (!setArg(tokens.at(iToken), cSecondaryAccuracy)) { incorrectArgs("cSecondaryAccuracy", iLine, iToken); return false; }
      checkRangeB(cSecondaryAccuracy, (uint32_t)1, (uint32_t)100);
      cMove.secondaryAccuracy = cSecondaryAccuracy;
    }

    // debuffs
    iToken = 18;
    for (size_t iBuff = 0; iBuff < 9; iBuff++)
    {
      // debuff, target
      int32_t tempBuff;
      if (!setArg(tokens.at(iToken + iBuff), tempBuff)) { incorrectArgs("debuff", iLine, iToken + iBuff); return false; }
      checkRangeB(tempBuff, (int32_t)-12, (int32_t)12);
      cMove.targetDebuff[iBuff] = tempBuff;
    }

    uint32_t ailment;
    cMove.targetAilment = AIL_NV_NONE;

    // ailment,target,burn
    iToken = 27;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetAilment = AIL_NV_BURN; goto endAilment; }
    }
    else { incorrectArgs("AIL_NV_BURN", iLine, iToken); return false; }

    // ailment,target,Freeze
    iToken = 28;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetAilment = AIL_NV_FREEZE; goto endAilment; }
    }
    else { incorrectArgs("AIL_NV_FREEZE", iLine, iToken); return false; }

    // ailment,target,paralysis
    iToken = 29;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetAilment = AIL_NV_PARALYSIS; goto endAilment; }
    }
    else { incorrectArgs("AIL_NV_PARALYSIS", iLine, iToken); return false; }

    // ailment,target,poison
    iToken = 30;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)2);
      if (ailment == 1) { cMove.targetAilment = AIL_NV_POISON; goto endAilment; }
      else if(ailment == 2) { cMove.targetAilment = AIL_NV_POISON_TOXIC; goto endAilment; }
    }
    else { incorrectArgs("AIL_NV_POISON/AIL_NV_POISON_TOXIC", iLine, iToken); return false; }

    // ailment,target,sleep
    iToken = 31;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetAilment = AIL_NV_SLEEP; goto endAilment; }
    }
    else { incorrectArgs("AIL_NV_SLEEP", iLine, iToken); return false; }

    endAilment:

    cMove.targetVolatileAilment = AIL_V_NONE;
    // volatileAilment,target,confusion
    iToken = 32;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetVolatileAilment = AIL_V_CONFUSED; goto endVolatile; }
    }
    else { incorrectArgs("AIL_V_CONFUSED", iLine, iToken); return false; }

    // volatileAilment,target,flinch
    iToken = 33;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetVolatileAilment = AIL_V_FLINCH; goto endVolatile; }
    }
    else { incorrectArgs("AIL_V_FLINCH", iLine, iToken); return false; }

    // volatileAilment,target,identify
    iToken = 34;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetVolatileAilment = 110; goto endVolatile; }
    }
    else { incorrectArgs("AIL_V_IDENTIFY", iLine, iToken); return false; }

    // volatileAilment,target,infatuation
    iToken = 35;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetVolatileAilment = AIL_V_INFATUATED; goto endVolatile; }
    }
    else { incorrectArgs("AIL_V_INFATUATED", iLine, iToken); return false; }

    // volatileAilment,target,lock on
    iToken = 36;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetVolatileAilment = 111; goto endVolatile; }
    }
    else { incorrectArgs("AIL_V_LOCKON", iLine, iToken); return false; }

    // volatileAilment,target,nightmare
    iToken = 37;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetVolatileAilment = 112; goto endVolatile; }
    }
    else { incorrectArgs("AIL_V_NIGHTMARE", iLine, iToken); return false; }

    // volatileAilment,target,partial trap
    iToken = 38;
    if (setArg(tokens.at(iToken), ailment))
    {
      checkRangeB(ailment, (uint32_t)0, (uint32_t)1);
      if (ailment == 1) { cMove.targetVolatileAilment = 113; goto endVolatile; }
    }
    else { incorrectArgs("AIL_V_PARTIALTRAP", iLine, iToken); return false; }

    endVolatile:

    //hasScript
    iToken = 39;
    if (tokens.at(iToken).compare("---") == 0)
    { cMove.setHasNoPlugins(); }

    //description
    iToken = 40;
    if (tokens.at(iToken).compare("---") == 0)
    { cMove.description.clear(); }
    else
    {
      size_t tokenLength = tokens.at(iToken).size();
      size_t offset = 0;
      //check for quotations, and if they exist remove them
      if (tokens.at(iToken)[0] == '"' && tokens.at(iToken)[tokenLength-1] == '"')
      {
        offset = 1;
      }

      cMove.description = std::string(tokens.at(iToken).substr(offset, tokenLength - offset));
    }
    if (verbose >= 6) std::cout << "\tLoaded move " << iMove << "-\"" << cMove.getName() << "\"\n";

  } //end of per-move

  if (mismatchedTypes.size() != 0)
  {
    if (verbose >= 5) std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
      ": move inputStream - " << mismatchedTypes.size() << " Orphaned move-types!\n";
    if (verbose >= 6)
    {
      for (size_t indexOrphan = 0; indexOrphan < mismatchedTypes.size(); indexOrphan++)
      {
        std::cout << "\tOrphaned type \"" << mismatchedTypes.at(indexOrphan) << "\"\n";
      }
    }
  }

  return true;
} // end of inputMoves
