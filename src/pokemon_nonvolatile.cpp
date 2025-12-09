#include "pokemonai/pokemon_nonvolatile.h"

#include <algorithm>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <numeric>

#include "pokemonai/pokedex.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/team_volatile.h"
#include "pokemonai/pokemon_base.h"
#include "pokemonai/nature.h"
#include "pokemonai/ability.h"
#include "pokemonai/item.h"
#include "pokemonai/move.h"

#include "pokemonai/init_toolbox.h"
#include "pokemonai/orphan.h"

using namespace orphan;
namespace pt = boost::property_tree;

std::array< std::array<fpType, 13>, 3> PokemonNonVolatile::aFV_base;

PokemonNonVolatile::PokemonNonVolatile() 
  : Name(),
  Signature<PokemonNonVolatile, POKEMON_NONVOLATILE_DIGESTSIZE>(),
  Serializable<PokemonNonVolatile>(),
  base_(PokemonBase::no_base),
  chosenAbility_(Ability::no_ability),
  chosenNature_(Nature::no_nature),
  initialItem_(Item::no_item),
  actions_(),
  level_(0),
  sex_(SEX_NEUTER) {
  // zero IV and EV
  IV_.fill(0);
  EV_.fill(0);
  
  // zero FV
  for (size_t iFV = 0; iFV < FV_base_.size(); iFV++)
  {
    FV_base_[iFV].fill(0);
  }
  
}


void PokemonNonVolatile::initialize() {
  for (auto& cMove: actions_) {
    // do not initialize if the move_nonvolatile object does not reference a valid move
    if (!cMove.moveExists()) { continue; }

    cMove.initialize(*this);
  }
  initFV();
};


void PokemonNonVolatile::uninitialize() {
  for (auto& cMove: actions_) {
    // do not initialize if the move_nonvolatile object does not reference a valid move
    if (!cMove.moveExists()) { continue; }

    cMove.uninitialize(*this);
  }
}


void PokemonNonVolatile::createDigest_impl(std::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE>& digest) const {
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
  pack(level_, digest, iDigest);

  // pack sex:
  pack(sex_, digest, iDigest);

  // pack IVEV:
  for (size_t iIEV = 0; iIEV < IV_.size(); ++iIEV)
  {
    pack(IV_[iIEV], digest, iDigest);
    pack(EV_[iIEV], digest, iDigest);
  }

  assert(iDigest == POKEMON_NONVOLATILE_DIGESTSIZE);
};


bool PokemonNonVolatile::pokemonExists() const {
  return (base_!=PokemonBase::no_base)?true:false;
};


PokemonNonVolatile& PokemonNonVolatile::setNoBase() {
  base_ = PokemonBase::no_base;
  setNoAbility();
  actions_.clear();

  return *this;
}


PokemonNonVolatile& PokemonNonVolatile::setBase(const PokemonBase& _base) {
  if (&_base == PokemonBase::no_base) { return setNoBase(); }
  // replace abilities / moves which are no longer valid with the new base:
  if (!isLegalAbility(getAbility())) { setNoAbility(); }
  for (size_t iMove = 0; iMove != getNumMoves(); ++iMove) {
    if (isLegalSet(iMove, getMove(iMove))) { continue; }
    removeMove(iMove);
    iMove--;
  }

  base_ = &_base;
  return *this;
};


PokemonNonVolatile& PokemonNonVolatile::setLevel(unsigned int _level) {
  level_ = _level;
  return *this;
};


PokemonNonVolatile& PokemonNonVolatile::setSex(unsigned int _sex) {
  if (!INI::checkRangeB(_sex, (uint32_t)0, (uint32_t)2)) {
    throw std::invalid_argument("PokemonNonVolatile sex");
  }

  sex_ = _sex;
  return *this;
};


PokemonNonVolatile& PokemonNonVolatile::setIV(size_t type, unsigned int value) {
  if (!INI::checkRangeB(value, (uint32_t)0, (uint32_t)31)) { 
    throw std::invalid_argument("PokemonNonVolatile IV");
  }
  IV_.at(type) = value;
  return *this;
}


PokemonNonVolatile& PokemonNonVolatile::setZeroEV() {
  EV_.fill(0);
  return *this;
}


PokemonNonVolatile& PokemonNonVolatile::setEV(size_t type, unsigned int value) {
  // validate value is within range:
  if (!INI::checkRangeB(value, (uint32_t)0, (uint32_t)255)) {
    throw std::invalid_argument("PokemonNonVolatile EV");
  }
  // validate array is within range:
  if (std::accumulate(EV_.begin(), EV_.end(), value) > (MAXEFFORTVALUE - getEV(type))) {
    throw std::invalid_argument("PokemonNonVolatile EV count");
  }

  EV_.at(type) = value;
  return *this;
}


bool PokemonNonVolatile::abilityExists() const {
  return (chosenAbility_!=Ability::no_ability)?true:false;
};


PokemonNonVolatile& PokemonNonVolatile::setAbility(const Ability& _chosenAbility) {
  if (&_chosenAbility == Ability::no_ability) { return setNoAbility(); }

  if(!isLegalAbility(_chosenAbility)) {
    throw std::invalid_argument("PokemonNonVolatile illegal ability");
  }

  chosenAbility_ = &_chosenAbility;
  return *this;
};


PokemonNonVolatile& PokemonNonVolatile::setNoAbility() {
  chosenAbility_ = Ability::no_ability;
  return *this;
};


PokemonNonVolatile& PokemonNonVolatile::setNoNature() {
  chosenNature_ = Nature::no_nature;
  return *this;
};


bool PokemonNonVolatile::natureExists() const {
  return (chosenNature_!=Nature::no_nature)?true:false;
};


PokemonNonVolatile& PokemonNonVolatile::setNature(const Nature& _chosenNature) {
  assert(pkdex->getNatures().count(_chosenNature.getName()) > 0);
  chosenNature_ = &_chosenNature;
  return *this;
};


PokemonNonVolatile& PokemonNonVolatile::setNoInitialItem() {
  initialItem_ = Item::no_item;
  return *this;
};


PokemonNonVolatile& PokemonNonVolatile::setInitialItem(const Item& _chosenItem) {
  if (&_chosenItem == Item::no_item) { return setNoInitialItem(); }

  assert(pkdex->getItems().count(_chosenItem.getName()) > 0);
  if (!_chosenItem.isImplemented()) {
    throw std::invalid_argument("PokemonNonVolatile item not implemented");
  }
  
  initialItem_ = &_chosenItem;
  return *this;
}


bool PokemonNonVolatile::hasInitialItem() const {
  return initialItem_ != Item::no_item;
}


const Item& PokemonNonVolatile::getInitialItem() const {
  return *initialItem_;
}


bool PokemonNonVolatile::isLegalAbility(const Ability& candidate) const {
  if (&candidate == Ability::no_ability) { return true; }

  assert(pkdex->getAbilities().count(candidate.getName()) > 0);
  if(!candidate.isImplemented()) {
    return false;
  }
  if (pokemonExists() && getBase().getAbilities().count(&candidate) == 0) {
    return false;
  }
  return true;
}


bool PokemonNonVolatile::isLegalAdd(const MoveNonVolatile& candidate) const {
  if ( !candidate.moveExists() ) { return false; }
  return isLegalAdd(candidate.getBase());
}


bool PokemonNonVolatile::isLegalAdd(const Move& candidate) const {
  if ((getNumMoves() + 1) > getMaxNumMoves()) { return false; }
  return isLegalSet(SIZE_MAX, candidate);
}


bool PokemonNonVolatile::isLegalSet(size_t iAction, const MoveNonVolatile& candidate) const {
  if ( !candidate.moveExists() ) { return false; }
  return isLegalSet(iAction, candidate.getBase());
}


bool PokemonNonVolatile::isLegalSet(size_t iAction, const Move& candidate) const {
  size_t iPosition = iAction;
  if (!pokemonExists()) { return false; }
  if ((iPosition != SIZE_MAX) && (iPosition >= getNumMoves()) ) { return false; }
  if ((candidate.lostChild == true) || (candidate.isImplemented() == false)) { return false; }

  // ensure that the move is within the pokemon's moveset
  const auto& cMovelist = getBase().moves_;
  if (!cMovelist.count(&candidate)) { return false; }

  // ensure that the move is not assigned multiple times to the same pokemon
  for (size_t iMove = 0; iMove != getNumMoves(); ++iMove) {
    if (iPosition == iMove) { continue; }
    if (&getMove_base(iMove) == &candidate) { return false; }
  }

  return true;
}


PokemonNonVolatile& PokemonNonVolatile::addMove(const MoveNonVolatile& _cMove) {
  if (!isLegalAdd(_cMove)) {
    throw std::invalid_argument("PokemonNonVolatile illegal move");
  }

  actions_.push_back(_cMove);
  return *this;
}


MoveNonVolatile& PokemonNonVolatile::getMove(size_t index) {
  switch(index) {
    default:
      assert(false && "attempted to get volatile move of non-move action");
    case 4:
      return *MoveNonVolatile::mNV_struggle;
    case 0:
    case 1:
    case 2:
    case 3:
      assert(index < getNumMoves());
      return actions_[index];
  }
};


const MoveNonVolatile& PokemonNonVolatile::getMove(size_t index) const {
  switch(index) {
    default:
      assert(false && "attempted to get volatile move of non-move action");
    case 4:
      return *MoveNonVolatile::mNV_struggle;
    case 0:
    case 1:
    case 2:
    case 3:
      assert(index < getNumMoves());
      return actions_[index];
  }
};


PokemonNonVolatile& PokemonNonVolatile::setMove(size_t iAction, const MoveNonVolatile& _cMove) {
  if (!isLegalSet(iAction, _cMove)) {
    throw std::invalid_argument("PokemonNonVolatile illegal move");
  }
  
  getMove(iAction) = _cMove;
  return *this;
};


PokemonNonVolatile& PokemonNonVolatile::removeMove(size_t iRemovedAction) {
  // don't bother removing a move that doesn't exist
  if (iRemovedAction >= getNumMoves()) { return *this; }
  actions_.erase(actions_.begin() + iRemovedAction);
  
  return *this;
};


void PokemonNonVolatile::setFV(unsigned int targetFV) {
  // set default value:
  if (targetFV == FV_HITPOINTS)
  {
    unsigned int baseStat = base_->baseStats_[targetFV];
    unsigned int iv = IV_[targetFV];
    unsigned int ev = EV_[targetFV];
    
    FV_base_[targetFV][STAGE0] = ((2 * baseStat + iv + (ev / 4)) * level_ / 100 + level_ + 10);
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
    unsigned int baseStat = base_->baseStats_[targetFV];
    unsigned int iv = IV_[targetFV];
    unsigned int ev = EV_[targetFV];
    unsigned int natureModification = chosenNature_->modTable_[targetFV];
    
    unsigned int base_FV = ((((2 * baseStat + iv + (ev / 4)) * level_ / 100 + 5) * natureModification) / FPMULTIPLIER);
    FV_base_[targetFV][STAGE0] = base_FV;
  }

  // set boosted values:
  for (size_t iBoost = 0; iBoost != 13; ++iBoost)
  {
    int boostStage = (int)iBoost - 6;

    if (boostStage == 0) { continue; } // don't modify base values

    if (targetFV == FV_HITPOINTS)
    {
      // hitpoints cannot be boosted
      FV_base_[targetFV][iBoost] = FV_base_[targetFV][STAGE0];
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

      FV_base_[targetFV][iBoost] = (FV_base_[targetFV][STAGE0] * boostNumerator) / boostDenominator;
    } // endOf atk, spa, def, spd, spe
  } // endOf foreach boost stage
} // endOf setFV


void PokemonNonVolatile::initFV() {
  // generate final values for a pokemon
  for (unsigned int indexFV = 0; indexFV < 9; indexFV++) {
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
static const std::array<std::string, 6> statHeaders = {"atk", "spa", "def", "spd", "spe", "hp"};


boost::property_tree::ptree PokemonNonVolatile::output(bool printHeader) const {
  pt::ptree result;
  if (printHeader) {
    result.put("header", header);
  }
  result.put("name", getName());
  result.put("species", getBase().getName());
  result.put("level", getLevel());
  result.put("item", getInitialItem().getName());
  result.put("sex", getSex());
  result.put("ability", getAbility().getName());
  result.put("nature", getNature().getName());

  // moves:
  pt::ptree& moves = result.put_child("moves", pt::ptree{});
  for (size_t iMove = 0; iMove < getNumMoves(); ++iMove) {
    pt::ptree move;
    move.put("", getMove_base(iMove).getName());
    moves.push_back(pt::ptree::value_type{"", move});
  }

  // IVs / EVs:
  pt::ptree& ivs = result.put_child("iv", pt::ptree{});
  pt::ptree& evs = result.put_child("ev", pt::ptree{});
  for (size_t iStat = 0; iStat < 6; ++iStat) {
    ivs.put(statHeaders[iStat], getIV(iStat));
    evs.put(statHeaders[iStat], getEV(iStat));
  }

  return result;
}


void PokemonNonVolatile::input(const pt::ptree& ptree) {
  Orphanage orphans;
  input(ptree, orphans);

  orphans.printAllOrphans(getName(), "team", 4);
}


void PokemonNonVolatile::input(const pt::ptree& ptree, Orphanage& orphanage) {
  if (ptree.count("pokemon") > 0) { return input(ptree.get_child("pokemon"), orphanage); }

  setName(ptree.get<std::string>("name"));
  setLevel(ptree.get<uint32_t>("level"));
  setSex(ptree.get<uint32_t>("sex"));

  const PokemonBase* species =
      orphanCheck(pkdex->getPokemon(), ptree.get<std::string>("species"), &orphanage.pokemon);
  setBase(*(species==NULL?PokemonBase::no_base:species));

  const Item* item =
      orphanCheck(pkdex->getItems(), ptree.get<std::string>("item"), &orphanage.items);
  setInitialItem(*(item==NULL?Item::no_item:item));

  const Ability* ability =
      orphanCheck(pkdex->getAbilities(), ptree.get<std::string>("ability"), &orphanage.abilities);
  try {
    setAbility(*(ability==NULL?Ability::no_ability:ability));
  } catch (std::invalid_argument& e) {
    if (verbose >= 5) {
      std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
        ": pokemon " << *this <<
        " cannot use illegal ability \"" << ability->getName() <<
        "\"!\n";
    }
  }

  const Nature* nature =
      orphanCheck(pkdex->getNatures(), ptree.get<std::string>("nature"), &orphanage.natures);
  setNature(*(nature==NULL?Nature::no_nature:nature));

  for (auto& e: ptree.get_child("moves")) {
    const Move* move =
      orphanCheck(pkdex->getMoves(), e.second.get<std::string>(""), &orphanage.moves);
    if (move == NULL) { continue; }
    try {
      addMove(*move);
    } catch (std::invalid_argument& e) {
      if (verbose >= 5) {
        std::cerr << "WAR " << __FILE__ << "." << __LINE__ <<
          ": pokemon " << *this <<
          " cannot use illegal move \"" << move->getName() <<
          "\"!\n";
      }
    }
  }

  const pt::ptree& ivs = ptree.get_child("iv");
  const pt::ptree& evs = ptree.get_child("ev");
  for (size_t iStat = 0; iStat < 6; ++iStat) {
    setIV(iStat, ivs.get<uint32_t>(statHeaders[iStat]));
    setEV(iStat, evs.get<uint32_t>(statHeaders[iStat]));
  }

  if (getNumMoves() == 0) {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": pokemon " << *this <<
      " does not have enough valid moves (" << getNumMoves() <<
      ")!\n";
    throw std::invalid_argument("PokemonNonVolatile numMoves");
  }
}
