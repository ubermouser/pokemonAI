#include "../inc/pokemon_nonvolatile.h"

#include <algorithm>
#include <boost/format.hpp>

#include "../inc/pokedex.h"
#include "../inc/pokemon_volatile.h"
#include "../inc/team_volatile.h"
#include "../inc/pokemon_base.h"
#include "../inc/nature.h"
#include "../inc/ability.h"
#include "../inc/item.h"
#include "../inc/move.h"

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"

using namespace orphan;

std::array< std::array<fpType, 13>, 3> PokemonNonVolatile::aFV_base;

PokemonNonVolatile::PokemonNonVolatile() 
  : Name(),
  Signature<PokemonNonVolatile, POKEMON_NONVOLATILE_DIGESTSIZE>(),
  base(PokemonBase::no_base),
  chosenAbility(Ability::no_ability), 
  chosenNature(Nature::no_nature),
  initialItem(Item::no_item),
  actions(),
  numMoves(0),
  level(0), 
  sex(SEX_NEUTER)
{	
  // zero IV and EV
  IV.fill(0);
  EV.fill(0);
  
  // zero FV
  for (size_t iFV = 0; iFV < FV_base.size(); iFV++)
  {
    FV_base[iFV].fill(0);
  }
  
}





PokemonNonVolatile::PokemonNonVolatile(const PokemonNonVolatile& orig) 
  : Name(orig),
  Signature<PokemonNonVolatile, POKEMON_NONVOLATILE_DIGESTSIZE>(orig),
  base(orig.base),
  chosenAbility(orig.chosenAbility), 
  chosenNature(orig.chosenNature), 
  actions(orig.actions),
  numMoves(orig.numMoves),
  initialItem(orig.initialItem),
  level(orig.level), 
  sex(orig.sex),
  IV(orig.IV),
  EV(orig.EV),
  FV_base(orig.FV_base)
{
}

PokemonNonVolatile& PokemonNonVolatile::operator=(const PokemonNonVolatile& other)
{
  // identity theorem - simply return what we have now if equal address
  if (this == &other) { return *this; } 
  
  Name::operator=(other);
  Signature<PokemonNonVolatile, POKEMON_NONVOLATILE_DIGESTSIZE>::operator=(other);
  base = other.base;
  chosenAbility = other.chosenAbility;
  chosenNature = other.chosenNature;
  actions = other.actions;
  numMoves = other.numMoves;
  initialItem = other.initialItem;
  level = other.level;
  sex = other.sex;
  IV = other.IV;
  EV = other.EV;
  FV_base = other.FV_base;
  
  return *this;
}





void PokemonNonVolatile::initialize()
{
  for (size_t iAction = 0; iAction < getNumMoves(); iAction++)
  {
    MoveNonVolatile& cMove = getMove(iAction);

    // do not initialize if the move_nonvolatile object does not reference a valid move
    if (!cMove.moveExists()) { continue; }

    cMove.initialize(*this);
  }
  initFV();
};





void PokemonNonVolatile::uninitialize()
{
  for (size_t iAction = 0; iAction < getNumMoves(); iAction++)
  {
    MoveNonVolatile& cMove = getMove(iAction);

    // do not initialize if the move_nonvolatile object does not reference a valid move
    if (!cMove.moveExists()) { continue; }

    cMove.uninitialize(*this);
  }
}





void PokemonNonVolatile::createDigest_impl(std::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE>& digest) const
{
  digest.fill(0);

  std::array<bool, 4> hashedMoves;
  hashedMoves.fill(false);
  size_t iDigest = 0;

  // hash by a useful order:
  for (size_t iOrder = 0, iSize = getNumMoves(); iOrder < iSize; ++iOrder)
  {
    size_t iBestMove = SIZE_MAX;
    const MoveNonVolatile* bestMove = NULL;
    for (size_t iAction = 0; iAction != iSize; ++iAction)
    {
      const MoveNonVolatile& cMove = getMove(iAction);

      // don't hash a move that has already been hashed:
      if (hashedMoves[iAction] == true) { continue; }

      // if no move has been selected yet, select the first move:
      if (bestMove == NULL) { bestMove = &cMove; iBestMove = iAction; continue; }

      // if bestMove appears later in the array of base moves than does cMove: (higher in alphabetical order)
      if (&bestMove->getBase() > &cMove.getBase()) { bestMove = &cMove; iBestMove = iAction; }
    }

    // no more moves to be hashed
    if (bestMove == NULL) { break; }

    hashedMoves[iBestMove] = true;

    // hash action:
    std::array<uint8_t, MOVE_NONVOLATILE_DIGESTSIZE> bMoveDigest;
    bestMove->createDigest(bMoveDigest);

    // copy action to pokemon digest:
    pack(bMoveDigest, digest, iDigest);
  } // endOf foreach moveOrdered
  iDigest = MOVE_NONVOLATILE_DIGESTSIZE * 4;

  
  if (pokemonExists())
  {
    // pack first 20 characters of name:
    getBase().getName().copy((char *)(digest.data() + iDigest), 20, 0);
  }
  iDigest += 20;

  if (abilityExists())
  { // pack 20 characters of ability:
    getAbility().getName().copy((char *)(digest.data() + iDigest), 20, 0);
  }
  iDigest += 20;

  if (natureExists())
  { // pack 20 characters of nature:
    getNature().getName().copy((char *)(digest.data() + iDigest), 20, 0);
  }
  iDigest += 20;

  if (hasInitialItem())
  {
    // pack 20 characters of item, if it exists:
    getInitialItem().getName().copy((char *)(digest.data() + iDigest), 20, 0);
  }
  iDigest += 20;

  // pack level:
  pack(level, digest, iDigest);

  // pack sex:
  pack(sex, digest, iDigest);

  // pack IVEV:
  for (size_t iIEV = 0; iIEV < IV.size(); ++iIEV)
  {
    pack(IV[iIEV], digest, iDigest);
    pack(EV[iIEV], digest, iDigest);
  }

  assert(iDigest == POKEMON_NONVOLATILE_DIGESTSIZE);
};





bool PokemonNonVolatile::pokemonExists() const
{
  return (base!=PokemonBase::no_base)?true:false;
};





bool PokemonNonVolatile::abilityExists() const
{
  return (chosenAbility!=Ability::no_ability)?true:false;
};





PokemonNonVolatile& PokemonNonVolatile::setAbility(const Ability& _chosenAbility)
{
  assert(pokemonExists());
  assert(_chosenAbility.isImplemented());
  assert(getBase().abilities_.count(&_chosenAbility) > 0);

  chosenAbility = &_chosenAbility;
  return *this;
};





PokemonNonVolatile& PokemonNonVolatile::setNoAbility()
{
  chosenAbility = Ability::no_ability;
  return *this;
};





PokemonNonVolatile& PokemonNonVolatile::setNoNature()
{
  chosenNature = Nature::no_nature;
  return *this;
};





bool PokemonNonVolatile::natureExists() const
{
  return (chosenNature!=Nature::no_nature)?true:false;
};





PokemonNonVolatile& PokemonNonVolatile::setNature(const Nature& _chosenNature)
{
  assert(pkdex->getNatures().count(_chosenNature.getName()) > 0);
  chosenNature = &_chosenNature;
  return *this;
};





PokemonNonVolatile& PokemonNonVolatile::setNoInitialItem()
{
  initialItem = Item::no_item;
  return *this;
};





PokemonNonVolatile& PokemonNonVolatile::setInitialItem(const Item& _chosenItem)
{
  assert(pkdex->getItems().count(_chosenItem.getName()) > 0);
  assert(_chosenItem.isImplemented());
  initialItem = &_chosenItem;
  return *this;
}





bool PokemonNonVolatile::hasInitialItem() const
{
  return initialItem != Item::no_item;
}





const Item& PokemonNonVolatile::getInitialItem() const
{
  return *initialItem;
}





bool PokemonNonVolatile::isLegalAdd(const MoveNonVolatile& candidate) const
{
  if ( !candidate.moveExists() ) { return false; }
  return isLegalAdd(candidate.getBase());
}






bool PokemonNonVolatile::isLegalSet(size_t iAction, const MoveNonVolatile& candidate) const
{
  if ( !candidate.moveExists() ) { return false; }
  return isLegalSet(iAction, candidate.getBase());
}





bool PokemonNonVolatile::isLegalAdd(const Move& candidate) const
{
  if ((getNumMoves() + 1) > getMaxNumMoves()) { return false; }
  return isLegalSet(SIZE_MAX, candidate);
}






bool PokemonNonVolatile::isLegalSet(size_t iAction, const Move& candidate) const
{
  size_t iPosition = iAction;
  if (!pokemonExists()) { return false; }
  if ((iPosition != SIZE_MAX) && (iPosition >= getNumMoves()) ) { return false; }
  if ((candidate.lostChild == true) || (candidate.isImplemented() == false)) { return false; }

  // ensure that the move is within the pokemon's moveset
  const auto& cMovelist = getBase().moves_;
  if (!cMovelist.count(&candidate)) { return false; }

  // ensure that the move is not assigned multiple times to the same pokemon
  for (size_t iMove = 0; iMove != getNumMoves(); ++iMove)
  {
    if (iPosition == iMove) { continue; }
    if (&getMove_base(iMove) == &candidate) { return false; }
  }

  return true;
}





PokemonNonVolatile& PokemonNonVolatile::addMove(const MoveNonVolatile& _cMove)
{
  assert(isLegalAdd(_cMove));

  actions[numMoves] = _cMove;
  numMoves++;
  return *this;
}




MoveNonVolatile& PokemonNonVolatile::getMove(size_t index)
{
  switch(index)
  {
    default:
      assert(false && "attempted to get volatile move of non-move action");
    case 4:
      return *MoveNonVolatile::mNV_struggle;
    case 0:
    case 1:
    case 2:
    case 3:
      assert(index < getNumMoves());
      return actions[index];
  }
};





const MoveNonVolatile& PokemonNonVolatile::getMove(size_t index) const
{
  switch(index)
  {
    default:
      assert(false && "attempted to get volatile move of non-move action");
    case 4:
      return *MoveNonVolatile::mNV_struggle;
    case 0:
    case 1:
    case 2:
    case 3:
      assert(index < getNumMoves());
      return actions[index];
  }
};





PokemonNonVolatile& PokemonNonVolatile::setMove(size_t iAction, const MoveNonVolatile& _cMove)
{
  assert(isLegalSet(iAction, _cMove));
  getMove(iAction) = _cMove;
  return *this;
};





PokemonNonVolatile& PokemonNonVolatile::removeMove(size_t iRemovedAction)
{
  // don't bother removing a move that doesn't exist
  if (iRemovedAction >= getNumMoves()) { return *this; }

  {
    MoveNonVolatile& removedMove = getMove(iRemovedAction);

    // remove move:
    removedMove = MoveNonVolatile();
  }

  // there's a "hole" in the contiguous move array now, so 
  // refactor moves above the removed move:
  for (size_t iSource = iRemovedAction + 1; iSource < getNumMoves(); iSource++) {
    MoveNonVolatile& source = getMove(iSource);

    for (size_t iNDestination = 0; iNDestination < iSource; iNDestination++)
    {
      size_t iDestination = (iSource - iNDestination - 1);

      MoveNonVolatile& destination = actions[iDestination];

      // don't replace a move that exists already
      if (destination.moveExists()) { continue; }

      // perform copy
      destination = source;
      // delete source (no duplicates)
      source = MoveNonVolatile();
      break;
    }
  }

  // update number of moves in move array:
  numMoves--;
  return *this;
};





void PokemonNonVolatile::setFV(unsigned int targetFV)
{	

  // set default value:
  if (targetFV == FV_HITPOINTS)
  {
    unsigned int baseStat = base->baseStats_[targetFV];
    unsigned int iv = IV[targetFV];
    unsigned int ev = EV[targetFV];
    
    FV_base[targetFV][STAGE0] = ((2 * baseStat + iv + (ev / 4)) * level / 100 + level + 10);
  }
  else if (targetFV == FV_ACCURACY || targetFV == FV_EVASION)
  {
    aFV_base[targetFV - 6][STAGE0] = 1.0; // 1.0
  }
  else if (targetFV == FV_CRITICALHIT)
  {
    // critical hit stage 1 is hardcoded
    aFV_base[targetFV - 6][STAGE0] =  0.0625; //(ACCURACY_EVASION_INTEGER * 0.0625);
  }
  else // for atk, spa, def, spd, spe
  {
    unsigned int baseStat = base->baseStats_[targetFV];
    unsigned int iv = IV[targetFV];
    unsigned int ev = EV[targetFV];
    unsigned int natureModification = chosenNature->modTable_[targetFV];
    
    unsigned int base_FV = ((((2 * baseStat + iv + (ev / 4)) * level / 100 + 5) * natureModification) / FPMULTIPLIER);
    FV_base[targetFV][STAGE0] = base_FV;
  }

  // set boosted values:
  for (size_t iBoost = 0; iBoost != 13; ++iBoost)
  {
    int boostStage = (int)iBoost - 6;

    if (boostStage == 0) { continue; } // don't modify base values

    if (targetFV == FV_HITPOINTS)
    {
      // hitpoints cannot be boosted
      FV_base[targetFV][iBoost] = FV_base[targetFV][STAGE0];
    }
    else if (targetFV == FV_ACCURACY || targetFV == FV_EVASION)
    {
      fpType boostNumerator = 1, boostDenominator = 1;
      if (boostStage >= 1)
      {
        boostNumerator = 3 + boostStage;
        boostDenominator = 3;
      }
      else // boostStage <= -1
      {
        boostNumerator = 3;
        boostDenominator = 3 - boostStage;
      }

      if (targetFV == FV_ACCURACY)
      {
        aFV_base[targetFV - 6][iBoost] = (aFV_base[targetFV - 6][STAGE0] * boostNumerator) / boostDenominator;
      }
      else // evasion is accuracy's modification flipped
      {
        aFV_base[targetFV - 6][iBoost] = (aFV_base[targetFV - 6][STAGE0] * boostDenominator) / boostNumerator;
      }
      
    }// endOf if FV_ACCURACY or FV_EVASION
    else if (targetFV == FV_CRITICALHIT)
    {
      // values of critical hit are hardcoded, and are always 0 when less than stage 0
      fpType boosted_FV;
    
      // values of critical hit hardcoded
      switch(boostStage)
      {
        default:
          boosted_FV = 0.0; // no critical hit possible
          break;
        case 0:
          boosted_FV = 0.0625; // ACCURACY_EVASION_INTEGER * .0625
          break;
        case 1:
          boosted_FV = 0.125; // ACCURACY_EVASION_INTEGER * .125
          break;
        case 2:
          boosted_FV = 0.25; // ACCURACY_EVASION_INTEGER * .25
          break;
        case 3:
          boosted_FV = (1.0/3.0); // ACCURACY_EVASION_INTEGER * .333
          break;
        case 4: // maximum stage for critical hit is 4
        case 5:
        case 6:
          boosted_FV = 0.5; // ACCURACY_EVASION_INTEGER * .5
          break;
      }
    
      aFV_base[targetFV - 6][iBoost] = boosted_FV;
    } // endOf if FV_CRITICALHIT
    else // for atk, spa, def, spd, spe
    {
      int32_t boostNumerator = 1, boostDenominator = 1;
      if (boostStage >= 1)
      {
        boostNumerator = 2 + boostStage;
        boostDenominator = 2;
      }
      else  // boostStage <= -1
      {
        boostNumerator = 2;
        boostDenominator = 2 - boostStage;
      }

      FV_base[targetFV][iBoost] = (FV_base[targetFV][STAGE0] * boostNumerator) / boostDenominator;
    } // endOf atk, spa, def, spd, spe
  } // endOf foreach boost stage
} // endOf setFV





void PokemonNonVolatile::initFV()
{
  // generate final values for a pokemon
  for (unsigned int indexFV = 0; indexFV < 9; indexFV++)
  {
    setFV(indexFV);
  }
}


const std::string& PokemonNonVolatile::defineName() {
  std::string capitalizedName = getBase().getName();
  if (capitalizedName.size() > 0) { capitalizedName[0] = std::toupper(capitalizedName[0]); }
  setName((boost::format("-x%06x_%.14s")
      % (hash() & 0xffffff)
      % capitalizedName).str());
  return getName();
}


std::ostream& operator <<(std::ostream& os, const PokemonNonVolatile& cPKNV) {
  os << "\"" << cPKNV.getName() << "\"-\"" << cPKNV.getBase().getName() << "\"";
  return os;
}


void PokemonNonVolatile::printSummary(std::ostream& os) const {
  os << *this
     << "  " << getFV_base(FV_HITPOINTS)
     << "HP  A[" << (abilityExists()?getAbility().getName():"")
     << "]  I[" << (hasInitialItem()?getInitialItem().getName():"")
     << "]  M[";
  for (size_t iMove = 0; iMove != getNumMoves(); ++iMove) {
    os << getMove_base(iMove).getName() << ((iMove+1)==getNumMoves()?"":", ");
  }
  os << "]";
}


static const std::string header = "PKAIP0";

void PokemonNonVolatile::output(std::ostream& oFile, bool printHeader) const
{
  // header:
  if (printHeader)
  {
    oFile << header << "\t";
  }

  /* PKAIP0 <nickname> <species> <level> <item> <gender> <ability> <nature> <hp.type> <hp.dmg> <move 1> <move 2> <move 3> <move 4> <atk.iv> <spatck.iv> <def.iv> <spdef.iv> <spd.iv> <hp.iv> <atk.ev> <spatck.ev> <def.ev> <spdef.ev> <spd.ev> <hp.ev> */
  oFile 
    << getName() <<
    "\t" << (pokemonExists()?getBase().getName():"NONE") <<
    "\t" << getLevel() <<
    "\t" << (hasInitialItem()?getInitialItem().getName():"NONE") <<
    "\t" << getSex() <<
    "\t" << (abilityExists()?getAbility().getName():"NONE") <<
    "\t" << getNature().getName() <<
    "\t";

  // moves:
  for (size_t iMove = 0; iMove != 4; ++iMove)
  {
    oFile << ((iMove < getNumMoves())?getMove_base(iMove).getName():"NONE") << "\t";
  }

  // IVs:
  for (size_t iIV = 0; iIV != 6; ++iIV)
  {
    oFile << getIV(iIV) << "\t";
  }

  // EVs:
  for (size_t iEV = 0; iEV != 6; ++iEV)
  {
    oFile << getEV(iEV) << "\t";
  }

  // end of line
  oFile << "\n";
} // endOf outputPokemon

bool PokemonNonVolatile::input(
    const std::vector<std::string>& lines, 
    size_t& iLine, 
    OrphanSet* mismatchedPokemon,
    OrphanSet* mismatchedItems,
    OrphanSet* mismatchedAbilities,
    OrphanSet* mismatchedNatures,
    OrphanSet* mismatchedMoves)
{
  // are the enough lines in the input stream:
  if ((lines.size() - iLine) < 1U)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": unexpected end of input stream at line " << iLine << "!\n";
    return false; 
  }

  // compare pokemon_nonvolatile header:
  if (lines.at(iLine).compare(0, header.size(), header) != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": pokemon_nonvolatile stream has header of type \"" << lines.at(0).substr(0, header.size()) << 
      "\" (needs to be \"" << header <<
      "\") and is incompatible with this program!\n";

    return false;
  }

  std::vector<std::string> tokens = INI::tokenize(lines.at(iLine), "\t");
    
  if (!INI::checkRangeB(tokens.size(), (size_t)24, (size_t)24)) { return false; }
    
  //teammate nickname
  size_t iToken = 1;
  setName(tokens.at(iToken).substr(0, 20));

  // find species index:
  iToken = 2;
  {
    const PokemonBase* cSpecies = orphanCheck(pkdex->getPokemon(), tokens.at(iToken), mismatchedPokemon);
    if (cSpecies == NULL) 
    { } //orphan!
    else 
    { setBase(*cSpecies); }
  }
    
  // teammate level
  iToken = 3;
  {
    uint32_t _level;
    if (!INI::setArgAndPrintError("pokemon_nonvolatile level", tokens.at(iToken), _level, iLine, iToken)) { return false; }
    if (!INI::checkRangeB(_level, (uint32_t)1, (uint32_t)100)) { return false; }
    setLevel(_level);
  }

  // find item index:
  iToken = 4;
  {
    const Item* cItem = NULL;

    if (tokens.at(iToken).compare("NONE") != 0)
    {
      cItem = orphanCheck(pkdex->getItems(), tokens.at(iToken), mismatchedItems);
    }
      
    if (cItem == NULL) { } // orphan!
    else if (!cItem->isImplemented())
    {
      if (mismatchedItems != NULL) { mismatchedItems->insert(lowerCase(tokens.at(iToken))); }
      if (verbose >= 5)
      {
        std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
          ": pokemon \"" << getBase().getName() <<
          "\" cannot use unimplemented item \"" << cItem->getName() <<
          "\"!\n";
      }
      cItem = NULL;
    }

    if (cItem == NULL)
    {
      setNoInitialItem();
    }
    else
    {
      setInitialItem(*cItem); 
    }
  }
    
  // teammate gender
  iToken = 5;
  {
    uint32_t _sex;
    if (!INI::setArgAndPrintError("pokemon_nonvolatile sex", tokens.at(iToken), _sex, iLine, iToken)) { return false; }
    if (!INI::checkRangeB(_sex, (uint32_t)0, (uint32_t)2)) { return false; }
    setSex(_sex);
  }

  // find ability index:
  iToken = 6;
  {
    const Ability* cAbility = NULL;

    if (pokemonExists())
    {
      if (tokens.at(iToken).compare("NONE") != 0)
      {
        cAbility = orphan::orphanCheck(pkdex->getAbilities(), tokens.at(iToken), mismatchedAbilities);
      }

      const PokemonBase& cBase = getBase();
      if (cAbility == NULL) { } // orphan!
      else if (!cBase.abilities_.count(cAbility))
      {
        if (mismatchedAbilities != NULL) { mismatchedAbilities->insert(lowerCase(tokens.at(iToken))); }
        if (verbose >= 5)
        {
          std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
            ": pokemon \"" << getBase().getName() <<
            "\" does not own ability \"" << cAbility->getName() <<
            "\"!\n";
        }
        cAbility = NULL;
      }
      else if (!cAbility->isImplemented())
      {
        if (mismatchedAbilities != NULL) { mismatchedAbilities->insert(lowerCase(tokens.at(iToken))); }
        if (verbose >= 5)
        {
          std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
            ": pokemon \"" << getBase().getName() <<
            "\" cannot use unimplemented ability \"" << cAbility->getName() <<
            "\"!\n";
        }
        cAbility = NULL;
      }
    } // endOf if pokemon exists
    if (cAbility != NULL)
    {
      setAbility(*cAbility);
    }
    else
    {
      setNoAbility();
    }
  }

  // find nature index:
  iToken = 7;
  {
    const Nature* cNature = NULL;

    if (tokens.at(iToken).compare("NONE") != 0)
    {
      cNature = orphanCheck(pkdex->getNatures(), tokens.at(iToken), mismatchedNatures);
    }

    if (cNature != NULL)
    {
      setNature(*cNature);
    }
    else
    {
      setNoNature();
    }
  }

  // find move indecies:
  iToken = 8;
  for (size_t iAction = 0; iAction < 4; iAction++)
  {
    if (tokens.at(iToken + iAction).compare("NONE") == 0)
    {
      continue;
    }
    const Move* cMove = orphanCheck(pkdex->getMoves(), tokens.at(iToken + iAction), mismatchedMoves);
    if (cMove == NULL) 
    { 
      if (verbose >= 5)
      {
        // printed out later
        std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
          ": pokemon \"" << getBase().getName() <<
          "\" has nonexistent move at index " << iAction <<
          ":\"" << tokens.at(iToken + iAction) <<
          "\"!\n";
      }
      continue;
    } //orphan!
#ifndef _ALLOWINVALIDTEAMS
    if (pokemonExists())
    {
      if (!isLegalAdd(*cMove)) // TODO: warning, asserts pokemonExists
      {
        if (mismatchedMoves != NULL) { mismatchedMoves->insert(lowerCase(tokens.at(iToken + iAction))); }
        if (verbose >= 5)
        {
          std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
            ": pokemon \"" << getBase().getName() <<
            "\" cannot use " << (!cMove->isImplemented()?"unimplemented":"illegal") <<
            " move at index " << iAction <<
            ":\"" << tokens.at(iToken + iAction) <<
            "\"!\n";
        }
        continue;
      }
    }
#endif
    // add move, only if all preconditions for its being added have been met:
    {
      addMove(MoveNonVolatile(*cMove)); 
    }
  } //end of move indecies

#ifndef _ALLOWINVALIDTEAMS
  if (getNumMoves() == 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": pokemon \"" << getName() <<
      "\" - \"" << getBase().getName() <<
      " does not have enough valid moves (" << getNumMoves() <<
      ")!\n";
    return false;
  }
#endif
    
  // input IV's and EV's:
  iToken = 12;
  unsigned int evAccumulator = 0;
  for (size_t ieValueIndex = 0; ieValueIndex < 6; ieValueIndex++)
  {
    uint32_t temp_IVEV;
    if (!INI::setArgAndPrintError("pokemon_nonvolatile IV", tokens.at(iToken + ieValueIndex), temp_IVEV, iLine, iToken + ieValueIndex)) { return false; }
    if (!INI::checkRangeB(temp_IVEV, (uint32_t)0, (uint32_t)31)) { return false; } 
    setIV(ieValueIndex, temp_IVEV);

    if (!INI::setArgAndPrintError("pokemon_nonvolatile EV", tokens.at(iToken + 6 + ieValueIndex), temp_IVEV, iLine, iToken + 6 + ieValueIndex)) { return false; }
    if (!INI::checkRangeB(temp_IVEV, (uint32_t)0, (uint32_t)255)) { return false; } 
    setEV(ieValueIndex, temp_IVEV);
      
    // if we haven't reached the max value yet:
    if (evAccumulator == UINT_MAX) { continue; }
      
    evAccumulator += temp_IVEV;
    if (evAccumulator > MAXEFFORTVALUE)
    {
      evAccumulator = UINT_MAX;
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": pokemon \"" << getName() <<
        "\" - \"" << getBase().getName() <<
        "\" has greater than maximum EV count of 510!\n";
      return false;
    }
  } // end of IV / EV

  iLine++;
  return pokemonExists();
} // endOf inputPokemon
