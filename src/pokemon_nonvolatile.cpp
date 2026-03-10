#include "pokemonai/pokemon_nonvolatile.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/fmt/ostr.h>

#include <algorithm>
#include <boost/property_tree/ptree.hpp>
#include <numeric>
#include <stdexcept>

#include "pokemonai/ability.h"
#include "pokemonai/init_toolbox.h"
#include "pokemonai/item.h"
#include "pokemonai/move.h"
#include "pokemonai/nature.h"
#include "pokemonai/orphan.h"
#include "pokemonai/pokedex.h"
#include "pokemonai/pokemon_base.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/team_volatile.h"

using namespace orphan;
namespace pt = boost::property_tree;

std::array<std::array<FixType, 13>, 3> PokemonNonVolatile::aFV_base;

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
  if (isLegalAbility(getAbility()) != AbilityLearnResult::SUCCESS) { setNoAbility(); }
  for (size_t iMove = 0; iMove != getNumMoves(); ++iMove) {
    if (isLegalSet(iMove, getMove(iMove)) == MoveLearnResult::SUCCESS) { continue; }
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


const std::string& PokemonNonVolatile::getName() const {
  if (Name::getName().empty() && pokemonExists()) {
    return getBase().getName();
  }
  return Name::getName();
}


PokemonNonVolatile& PokemonNonVolatile::setAbility(const Ability& _chosenAbility) {
  if (&_chosenAbility == Ability::no_ability) { return setNoAbility(); }

  AbilityLearnResult result = isLegalAbility(_chosenAbility);
  handleAbilityLearnResult(result, _chosenAbility);

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
  assert(
      &_chosenNature == Nature::no_nature || 
      pkdex->getNatures().count(_chosenNature.getName()) > 0);
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
    throw std::invalid_argument(
        "PokemonNonVolatile item \"" + _chosenItem.getName() +
        "\" not implemented");
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


AbilityLearnResult PokemonNonVolatile::isLegalAbility(const Ability& candidate) const {
  if (&candidate == Ability::no_ability) { return AbilityLearnResult::SUCCESS; }

  if (pkdex->getAbilities().count(candidate.getName()) == 0) {
    return AbilityLearnResult::ABILITY_NOT_IN_POKEDEX;
  }
  if (!candidate.isImplemented()) {
    return AbilityLearnResult::ABILITY_NOT_IMPLEMENTED;
  }
  if (!pokemonExists()) {
    return AbilityLearnResult::POKEMON_DOES_NOT_EXIST;
  }
  if (getBase().getAbilities().count(&candidate) == 0) {
    return AbilityLearnResult::INVALID_ABILITY_FOR_SPECIES;
  }
  return AbilityLearnResult::SUCCESS;
}


MoveLearnResult PokemonNonVolatile::isLegalAdd(const MoveNonVolatile& candidate) const {
  if ( !candidate.moveExists() ) { return MoveLearnResult::MOVE_NOT_IN_MOVELIST; }
  return isLegalAdd(candidate.getBase());
}


MoveLearnResult PokemonNonVolatile::isLegalAdd(const Move& candidate) const {
  if ((getNumMoves() + 1) > getMaxNumMoves()) { return MoveLearnResult::MAX_MOVES_REACHED; }
  return isLegalSet(SIZE_MAX, candidate);
}


MoveLearnResult PokemonNonVolatile::isLegalSet(size_t iAction, const MoveNonVolatile& candidate) const {
  if ( !candidate.moveExists() ) { return MoveLearnResult::MOVE_NOT_IN_MOVELIST; }
  return isLegalSet(iAction, candidate.getBase());
}


MoveLearnResult PokemonNonVolatile::isLegalSet(size_t iAction, const Move& candidate) const {
  size_t iPosition = iAction;
  if (!pokemonExists()) { return MoveLearnResult::POKEMON_DOES_NOT_EXIST; }
  if ((iPosition != SIZE_MAX) && (iPosition >= getNumMoves()) ) { return MoveLearnResult::INVALID_MOVE_INDEX; }
  if ((candidate.lostChild == true) || (candidate.isImplemented() == false)) {
    return MoveLearnResult::MOVE_NOT_IMPLEMENTED;
  }

  // ensure that the move is within the pokemon's moveset
  const auto& cMovelist = getBase().moves_;
  if (!cMovelist.count(&candidate)) {
    return MoveLearnResult::MOVE_NOT_IN_MOVELIST;
  }

  // ensure that the move is not assigned multiple times to the same pokemon
  for (size_t iMove = 0; iMove != getNumMoves(); ++iMove) {
    if (iPosition == iMove) { continue; }
    if (&getMove_base(iMove) == &candidate) { return MoveLearnResult::MOVE_ALREADY_KNOWN; }
  }

  return MoveLearnResult::SUCCESS;
}

void PokemonNonVolatile::handleAbilityLearnResult(AbilityLearnResult result, const Ability& candidate) const {
  if (result == AbilityLearnResult::SUCCESS) { return; }

  const auto& ability_name = candidate.getName();
  const auto& species_name = this->getBase().getName();

  std::string errorMessage;
  switch (result) {
  case AbilityLearnResult::POKEMON_DOES_NOT_EXIST:
    errorMessage = "Pokemon \"" + getName() + "\" has no base class";
    break;
  case AbilityLearnResult::ABILITY_NOT_IMPLEMENTED:
    errorMessage = "Ability '" + ability_name + "' is not implemented / has no base class";
    break;
  case AbilityLearnResult::ABILITY_NOT_IN_POKEDEX:
    errorMessage = "Ability " + ability_name + " is not in the pokedex";
    break;
  case AbilityLearnResult::INVALID_ABILITY_FOR_SPECIES:
    errorMessage = "Ability " + ability_name + " is not a valid ability for species " + species_name;
    break;
  default:
    errorMessage = "Unknown error";
    break;
  }

  if (pkdex->allowInvalidPokemon()) {
    SPDLOG_WARN("Ignoring illegal ability for {}: {}", species_name, errorMessage);
  } else {
    throw std::invalid_argument(errorMessage);
  }
}

void PokemonNonVolatile::handleMoveLearnResult(MoveLearnResult result, const MoveNonVolatile& candidate) const {
  if (result == MoveLearnResult::SUCCESS) { return; }

  const auto& move_name = candidate.getBase().getName();
  const auto& species_name = this->getBase().getName();

  std::string errorMessage;
  switch (result) {
  case MoveLearnResult::POKEMON_DOES_NOT_EXIST:
    errorMessage = "Pokemon \"" + getName() + "\" has no base class";
    break;
  case MoveLearnResult::MAX_MOVES_REACHED:
    errorMessage = "Pokemon \"" + getName() + "\" cannot learn more than 4 moves";
    break;
  case MoveLearnResult::INVALID_MOVE_INDEX:
    errorMessage = "Invalid move index";
    break;
  case MoveLearnResult::MOVE_NOT_IMPLEMENTED:
    errorMessage = "Move " + move_name + " is not implemented / has no base class";
    break;
  case MoveLearnResult::MOVE_NOT_IN_MOVELIST:
    errorMessage = "Move " + move_name + " is not in species " + species_name + "'s movelist";
    break;
  case MoveLearnResult::MOVE_ALREADY_KNOWN:
    errorMessage = "Pokemon n=" + getName() + " already knows move" + move_name;
    break;
  default:
    errorMessage = "Unknown error";
    break;
  }

  if (pkdex->allowInvalidPokemon()) {
    SPDLOG_WARN("Ignoring illegal move for {}: {}", species_name, errorMessage);
  } else {
    throw std::invalid_argument(errorMessage);
  }
}

PokemonNonVolatile& PokemonNonVolatile::addMove(const MoveNonVolatile& _cMove) {
  MoveLearnResult result = isLegalAdd(_cMove);
  handleMoveLearnResult(result, _cMove);

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
  MoveLearnResult result = isLegalSet(iAction, _cMove);
  if (result != MoveLearnResult::SUCCESS) {
    handleMoveLearnResult(result, _cMove);
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
    aFV_base[targetFV - 6][STAGE0] = FixType(1.0f);  // 1.0
  } else if (targetFV == FV_CRITICALHIT) {
    // critical hit stage 1 is hardcoded
    aFV_base[targetFV - 6][STAGE0] =
        FixType(0.0625f);  //(ACCURACY_EVASION_INTEGER * 0.0625);
  } else                   // for atk, spa, def, spd, spe
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
      double boostMultiplier;
      if (boostStage >= 1)
      {
        boostMultiplier = (double)(3 + boostStage) / 3.0;
      }
      else // boostStage <= -1
      {
        boostMultiplier = 3.0 / (double)(3 - boostStage);
      }

      if (targetFV == FV_ACCURACY)
      {
        aFV_base[targetFV - 6][iBoost] = FixType(
            aFV_base[targetFV - 6][STAGE0].to_double() * boostMultiplier);
      }
      else // evasion is accuracy's modification flipped
      {
        aFV_base[targetFV - 6][iBoost] = FixType(
            aFV_base[targetFV - 6][STAGE0].to_double() / boostMultiplier);
      }
    }// endOf if FV_ACCURACY or FV_EVASION
    else if (targetFV == FV_CRITICALHIT)
    {
      // values of critical hit are hardcoded, and are always 0 when less than stage 0
      FixType boosted_FV;

      // values of critical hit hardcoded
      switch(boostStage)
      {
        default:
          boosted_FV = FixType(0.0);  // no critical hit possible
          break;
        case 0:
          boosted_FV = FixType(0.0625);  // ACCURACY_EVASION_INTEGER * .0625
          break;
        case 1:
          boosted_FV = FixType(0.125);  // ACCURACY_EVASION_INTEGER * .125
          break;
        case 2:
          boosted_FV = FixType(0.25);  // ACCURACY_EVASION_INTEGER * .25
          break;
        case 3:
          boosted_FV = FixType(1.0 / 3.0);  // ACCURACY_EVASION_INTEGER * .333
          break;
        case 4: // maximum stage for critical hit is 4
        case 5:
        case 6:
          boosted_FV = FixType(0.5);  // ACCURACY_EVASION_INTEGER * .5
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
  setName(fmt::format("-x{:06x}_{:.14}", hash() & 0xffffff, capitalizedName));
  return getName();
}


std::ostream& operator <<(std::ostream& os, const PokemonNonVolatile& cPKNV) {
  os << "\"" << cPKNV.getName() << "\"-\"" << cPKNV.getBase().getName() << "\"";
  return os;
}


void PokemonNonVolatile::printSummary(std::ostream& os) const {
  os << fmt::format(
      "{}  {}HP  A[{}]  I[{}]  M[",
      fmt::streamed(*this),
      getMaxHP(),
      (abilityExists() ? getAbility().getName() : ""),
      (hasInitialItem() ? getInitialItem().getName() : ""));
  for (size_t iMove = 0; iMove != getNumMoves(); ++iMove) {
    os << fmt::format(
        "{}{}",
        getMove_base(iMove).getName(),
        ((iMove + 1) == getNumMoves() ? "" : ", "));
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

  orphans.printAllOrphans(getName(), "team");
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
  try {
    setInitialItem(*(item==NULL?Item::no_item:item));
  } catch (const std::exception& e) {
    SPDLOG_ERROR(
        "pokemon \"{}\" cannot use item \"{}\": {}!",
        fmt::streamed(*this),
        item->getName(),
        e.what());
  }

  const Ability* ability =
      orphanCheck(pkdex->getAbilities(), ptree.get<std::string>("ability"), &orphanage.abilities);
  try {
    setAbility(*(ability==NULL?Ability::no_ability:ability));
  } catch (const std::exception& e) {
    SPDLOG_ERROR(
        "pokemon \"{}\" cannot use ability \"{}\": {}!",
        fmt::streamed(*this),
        ability->getName(),
        e.what());
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
    } catch (const std::exception& e) {
      SPDLOG_ERROR(
          "pokemon \"{}\" cannot use move \"{}\": {}!",
          fmt::streamed(*this),
          move->getName(),
          e.what());
    }
  }

  const pt::ptree& ivs = ptree.get_child("iv");
  const pt::ptree& evs = ptree.get_child("ev");
  for (size_t iStat = 0; iStat < 6; ++iStat) {
    setIV(iStat, ivs.get<uint32_t>(statHeaders[iStat]));
    setEV(iStat, evs.get<uint32_t>(statHeaders[iStat]));
  }

  if (getNumMoves() == 1) {
    std::stringstream out;
    out << "pokemon " << *this << " does not have enough valid moves (" << getNumMoves() << ")!";
    SPDLOG_ERROR("{}", out.str());
  }
}
