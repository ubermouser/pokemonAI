#include "../inc/pokemon_volatile.h"

#include "../inc/pokedex.h"
#include "../inc/name.h"
#include "../inc/pokemon_base.h"
#include "../inc/move.h"
#include "../inc/item.h"
#include "../inc/ability.h"
#include "../inc/nature.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/move_volatile.h"

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(PokemonVolatileData) == sizeof(uint64_t));


bool PokemonVolatileData::operator ==(const PokemonVolatileData& other) const {
  return std::memcmp(this, &other, sizeof(PokemonVolatileData)) == 0;
}


bool PokemonVolatileData::operator !=(const PokemonVolatileData& other) const {
  return !(*this == other);
}


void PokemonVolatile::initialize(bool isActive)
{
  // reassign initial item
  data().iHeldItem = nv().initialItem->index_;
  
  // raise HP back to normal
  data().HPcurrent = nv().getFV_base(FV_HITPOINTS);

  // if lead pokemon, set isLead to true
  data().active = isActive;
  
  // reset volatile moves:
  for (size_t iMove = 0, iSize = nv().getNumMoves(); iMove != iSize; ++iMove)
  {
    // reset action
    getMV(AT_MOVE_0 + iMove).initialize();
  }
}


POKEMON_VOLATILE_IMPL_TEMPLATE
bool POKEMON_VOLATILE_IMPL::hasPP() const {
  bool result = true;

  result =
    (getMV(AT_MOVE_0).hasPP()) ||
    (getMV(AT_MOVE_1).hasPP()) ||
    (getMV(AT_MOVE_2).hasPP()) ||
    (getMV(AT_MOVE_3).hasPP());

  return result;
}


void PokemonVolatile::modHP(int32_t quantity) {
  int32_t _HP = data().HPcurrent + quantity; // an integer type so std::max will accept 0 if _HP is below 0
  
  data().HPcurrent = (uint16_t) std::min((uint32_t)std::max(_HP,0), nv().getFV_base(FV_HITPOINTS));
  
  // this pokemon has died, standardize its stats so comparisons work better - they're not important anymore
  if (!isAlive()) 
  { 
    // completely zero the pokemon
    data() = PokemonVolatileData();
    status().cTeammate = VolatileStatus();
  }
}


void PokemonVolatile::setHP(uint32_t _HP) {
  data().HPcurrent = (uint16_t) std::min((uint32_t)std::max(_HP,0U), nv().getFV_base(FV_HITPOINTS));
  
  // this pokemon has died, standardize its stats so comparisons work better - they're not important anymore
  if (!isAlive()) 
  { 
    // completely zero the pokemon and teammate status
    data() = PokemonVolatileData();
    status().cTeammate = VolatileStatus();
  }
}


void PokemonVolatile::modPercentHP(fpType percent) {
  int32_t quantity = percent * (fpType)nv().getFV_base(FV_HITPOINTS);
  modHP(quantity);
}


void PokemonVolatile::setPercentHP(fpType percent) {
  uint32_t quantity = percent * (fpType)nv().getFV_base(FV_HITPOINTS);
  setHP(quantity);
}


void PokemonVolatile::setStatusAilment(uint32_t statusCondition) {
  data().status_nonvolatile = statusCondition;
}


void PokemonVolatile::clearStatusAilment() {
  data().status_nonvolatile = AIL_NV_NONE;
}


POKEMON_VOLATILE_IMPL_TEMPLATE
bool POKEMON_VOLATILE_IMPL::isAlive() const {
  return data().HPcurrent > 0;
}


static MoveVolatileData standardMove;
POKEMON_VOLATILE_IMPL_TEMPLATE
typename POKEMON_VOLATILE_IMPL::movevolatile_t
POKEMON_VOLATILE_IMPL::getMV(size_t index) const {
  switch(index)
  {
  case AT_MOVE_0:
  case AT_MOVE_1:
  case AT_MOVE_2:
  case AT_MOVE_3:
    return movevolatile_t{
        nv().actions[index - AT_MOVE_0],
        data().actions[index - AT_MOVE_0]};
  default:
    assert(false && "attempted to access an unknown non volatile move!\n");
  case AT_MOVE_STRUGGLE:
    // TODO(@drendleman) - assert struggle cannot be retrieved from non-const context
    return movevolatile_t{*MoveNonVolatile::mNV_struggle, standardMove};
  };
}


POKEMON_VOLATILE_IMPL_TEMPLATE
fpType POKEMON_VOLATILE_IMPL::getPercentHP() const {
  return ((fpType) data().HPcurrent) / ((fpType)nv().getFV_base(FV_HITPOINTS));
}


POKEMON_VOLATILE_IMPL_TEMPLATE
uint32_t POKEMON_VOLATILE_IMPL::getHP() const {
  return data().HPcurrent;
}


POKEMON_VOLATILE_IMPL_TEMPLATE
const Item& POKEMON_VOLATILE_IMPL::getItem() const {
  assert(hasItem());
  return *pkdex->getItems().atByIndex(data().iHeldItem);
}


void PokemonVolatile::setNoItem(bool resetVolatile) {
  // TODO(@drendleman) index_ of no_item can be greater than the largest possible iHeldItem!
  data().iHeldItem = Item::no_item->index_;
  // item state is reset when the item is swapped out:
  if (resetVolatile) { status().cTeammate.itemScratch = 0; }
  assert(!hasItem()); // may not be true because of narrowing
}


POKEMON_VOLATILE_IMPL_TEMPLATE
bool POKEMON_VOLATILE_IMPL::hasItem() const {
  return data().iHeldItem != Item::no_item->index_;
}


POKEMON_VOLATILE_IMPL_TEMPLATE
int32_t POKEMON_VOLATILE_IMPL::getBoost(size_t type) const
{
  switch(type)
  {
  case FV_ATTACK:
    return status_->cTeammate.boosts.B_ATK;
  case FV_DEFENSE:
    return status_->cTeammate.boosts.B_DEF;
  case FV_SPATTACK:
    return status_->cTeammate.boosts.B_SPA;
  case FV_SPDEFENSE:
    return status_->cTeammate.boosts.B_SPD;
  case FV_SPEED:
    return status_->cTeammate.boosts.B_SPE;
  case FV_ACCURACY:
    return status_->cTeammate.boosts.B_ACC;
  case FV_EVASION:
    return status_->cTeammate.boosts.B_EVA;
  case FV_CRITICALHIT:
    return status_->cTeammate.boosts.B_CHT;
  default:
  case FV_HITPOINTS:
    return 0;
  }
};


void PokemonVolatile::setBoost(size_t type, int32_t value)
{
  switch(type)
  {
  case FV_ATTACK:
    status_->cTeammate.boosts.B_ATK = value; return;
  case FV_DEFENSE:
    status_->cTeammate.boosts.B_DEF = value; return;
  case FV_SPATTACK:
    status_->cTeammate.boosts.B_SPA = value; return;
  case FV_SPDEFENSE:
    status_->cTeammate.boosts.B_SPD = value; return;
  case FV_SPEED:
    status_->cTeammate.boosts.B_SPE = value; return;
  case FV_ACCURACY:
    status_->cTeammate.boosts.B_ACC = value; return;
  case FV_EVASION:
    status_->cTeammate.boosts.B_EVA = value; return;
  case FV_CRITICALHIT:
    status_->cTeammate.boosts.B_CHT = value; return;
  default:
  case FV_HITPOINTS:
    return;
  }
};


bool PokemonVolatile::modBoost(size_t type, int quantity) {
  int32_t oldbuff = getBoost(type);
  int32_t newbuff = std::min(std::max(oldbuff + quantity, -6), 6);

  // update FV value
  if (oldbuff != newbuff) {
    // there was an increment / decrement of buff and it was within the range of acceptable values
    setBoost(type, newbuff);
    // true if a change was made
    return true;
  } else {
    // false if no change made
    return false;
  }
}


POKEMON_VOLATILE_IMPL_TEMPLATE
uint32_t POKEMON_VOLATILE_IMPL::getFV_boosted(size_t type, int32_t tempBoost) const {
  int32_t cBoost = getBoost(type) + tempBoost;
  cBoost = std::min(std::max(cBoost, -6), 6);
  return nv().FV_base[type][cBoost + 6];
}


POKEMON_VOLATILE_IMPL_TEMPLATE
fpType POKEMON_VOLATILE_IMPL::getAccuracy_boosted(size_t type, int32_t tempBoost) const {
  int32_t cBoost = getBoost(type) + tempBoost;
  cBoost = std::min(std::max(cBoost, -6), 6);
  return PokemonNonVolatile::aFV_base[type - 6][cBoost + 6];
}


std::ostream& operator <<(std::ostream& os, const ConstPokemonVolatile& pkmn)
{
  os << "\"" << pkmn.nv().getName()
     << "\"-\"" << pkmn.nv().getBase().getName()
     << "\" " << pkmn.getHP()
     << "/" << pkmn.getFV_boosted(FV_HITPOINTS);

  // non-volatile ailments
  switch(pkmn.getStatusAilment())
  {
    case AIL_NV_BURN:
      os << " BRN";
      break;
    case AIL_NV_FREEZE:
      os << " FRZ";
      break;
    case AIL_NV_PARALYSIS:
      os << " PAR";
      break;
    case AIL_NV_POISON_TOXIC:
      os << " PST";
      break;
    case AIL_NV_POISON:
      os << " PSN";
      break;
    case AIL_NV_REST_1T:
    case AIL_NV_REST_2T:
    case AIL_NV_REST_3T:
    case AIL_NV_SLEEP_4T:
    case AIL_NV_SLEEP_3T:
    case AIL_NV_SLEEP_2T:
    case AIL_NV_SLEEP_1T:
      os << " SLP";
      break;
    case AIL_NV_NONE:
    default:
      break;
  }

  // boosts:
  if (pkmn.data().active) {
    // volatile ailments:
    // target confused:
    if (pkmn.status().cTeammate.confused > 0) {
      os << " (CNFSD)";
    }
    // target infatuated:
    if (pkmn.status().cTeammate.infatuate > 0) {
      os << " (INFAT)";
    }
    /*// target flinched:
    sA_V = AIL_V_FLINCH;
    if (cP.currentPokemon.getVolatileStatusCondition(cP.cPokemon, sA_V))
    {
      os << " (FLNCH)";
    }
    // target flinched last turn:
    sA_V = AIL_V_FLINCHED;
    if (cP.currentPokemon.getVolatileStatusCondition(cP.cPokemon, sA_V))
    {
      os << " (FLNCH)";
    }*/

    os << std::showpos; // show the + or -
    if (pkmn.getBoost(FV_ATTACK) != 0) {
      os << " " << pkmn.getBoost(FV_ATTACK) << "atk";
    }
    if (pkmn.getBoost(FV_SPATTACK) != 0) {
      os << " " << pkmn.getBoost(FV_SPATTACK) << "spa";
    }
    if (pkmn.getBoost(FV_DEFENSE) != 0) {
      os << " " << pkmn.getBoost(FV_DEFENSE) << "def";
    }
    if (pkmn.getBoost(FV_SPDEFENSE) != 0) {
      os << " " << pkmn.getBoost(FV_SPDEFENSE) << "spd";
    }
    if (pkmn.getBoost(FV_SPEED) != 0) {
      os << " " << pkmn.getBoost(FV_SPEED) << "spe";
    }
    if (pkmn.getBoost(FV_EVASION) != 0) {
      os << " " << pkmn.getBoost(FV_EVASION) << "eva";
    }
    if (pkmn.getBoost(FV_ACCURACY) != 0) {
      os << " " << pkmn.getBoost(FV_ACCURACY) << "acc";
    }
    if (pkmn.getBoost(FV_CRITICALHIT) != 0) {
      os << " " << pkmn.getBoost(FV_CRITICALHIT) << "crt";
    }
    os << std::noshowpos; // stop showing + or -
  }

  os << "\n";

  return os;
}


template class PokemonVolatileImpl<ConstMoveVolatile, const PokemonVolatileData, const TeamStatus>;
template class PokemonVolatileImpl<MoveVolatile, PokemonVolatileData, TeamStatus>;