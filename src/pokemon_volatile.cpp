
//#define PKAI_EXPORT
#include "../inc/pokemon_volatile.h"
//#undef PKAI_EXPORT

//#define PKAI_STATIC
#include "../inc/pokedex.h"
#include "../inc/name.h"
#include "../inc/pokemon_base.h"
#include "../inc/move.h"
#include "../inc/item.h"
#include "../inc/ability.h"
#include "../inc/nature.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/move_volatile.h"
//#undef PKAI_STATIC

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(pokemon_volatile) == sizeof(uint64_t));





bool pokemon_volatile::operator ==(const pokemon_volatile& other) const
{		
  if (raw != other.raw) { return false; }
  
  return true;
}





bool pokemon_volatile::operator !=(const pokemon_volatile& other) const
{
  return !(*this == other);
}





void pokemon_volatile::initialize(const pokemon_nonvolatile& nonvolatile)
{
  // zero data: (zeroed from context environment_volatile)
  //raw = 0;

  // reassign initial item
  data.iHeldItem = nonvolatile.hasInitialItem()?(nonvolatile.initialItem+1):0;
  
  // raise HP back to normal
  data.HPcurrent = nonvolatile.getFV_base(FV_HITPOINTS);
  
  // reset volatile moves:
  for (size_t iMove = 0, iSize = nonvolatile.getNumMoves(); iMove != iSize; ++iMove)
  {
    // reset action
    data.actions[iMove].initialize(nonvolatile.getMove(iMove + AT_MOVE_0));
  }
}





void pokemon_volatile::modHP(const pokemon_nonvolatile& nv, int32_t quantity)
{
  int32_t _HP = data.HPcurrent + quantity; // an integer type so std::max will accept 0 if _HP is below 0
  
  data.HPcurrent = (uint16_t) std::min((uint32_t)std::max(_HP,0), nv.getFV_base(FV_HITPOINTS));
  
  // this pokemon has died, standardize its stats so comparisons work better - they're not important anymore
  if (!isAlive()) 
  { 
    // completely zero the pokemon
    raw = 0;
  }
}





void pokemon_volatile::setHP(const pokemon_nonvolatile& nv, uint32_t _HP)
{
  
  data.HPcurrent = (uint16_t) std::min((uint32_t)std::max(_HP,0U), nv.getFV_base(FV_HITPOINTS));
  
  // this pokemon has died, standardize its stats so comparisons work better - they're not important anymore
  if (!isAlive()) 
  { 
    // completely zero the pokemon
    raw = 0;
  }
}





void pokemon_volatile::modPercentHP(const pokemon_nonvolatile& nv, fpType percent)
{
  int32_t quantity = percent * (fpType)nv.getFV_base(FV_HITPOINTS);
  
  modHP(nv, quantity);
}




void pokemon_volatile::setPercentHP(const pokemon_nonvolatile& nv, fpType percent)
{
  uint32_t quantity = percent * (fpType)nv.getFV_base(FV_HITPOINTS);

  setHP(nv, quantity);
}





void pokemon_volatile::setStatusAilment(uint32_t statusCondition)
{
  data.status_nonvolatile = statusCondition;
}





void pokemon_volatile::clearStatusAilment()
{
  data.status_nonvolatile = AIL_NV_NONE;
}





bool pokemon_volatile::isAlive() const
{
  return (data.HPcurrent>0);
}




static move_volatile standardMove = { 0 };
const move_volatile& pokemon_volatile::getMV(size_t index) const
{
  switch(index)
  {
  case AT_MOVE_0:
  case AT_MOVE_1:
  case AT_MOVE_2:
  case AT_MOVE_3:
    return data.actions[index - AT_MOVE_0];
  default:
    assert(false && "attempted to access an unknown non volatile move!\n");
  case AT_MOVE_STRUGGLE:
    return standardMove;
  };
}





move_volatile& pokemon_volatile::getMV(size_t index)
{
  switch(index)
  {
  case AT_MOVE_0:
  case AT_MOVE_1:
  case AT_MOVE_2:
  case AT_MOVE_3:
    return data.actions[index - AT_MOVE_0];
  default:
    assert(false && "attempted to access an unknown non volatile move!\n");
  case AT_MOVE_STRUGGLE:
    assert(false && "attempted to access a shared move in a non-const context!\n");
    return standardMove;
  };
}





fpType pokemon_volatile::getPercentHP(const pokemon_nonvolatile& nv) const
{
  return ((fpType) data.HPcurrent) / ((fpType)nv.getFV_base(FV_HITPOINTS));
}





uint32_t pokemon_volatile::getHP() const
{
  return data.HPcurrent;
}





const item& pokemon_volatile::getItem(const pokemon_nonvolatile& nv) const
{
  assert(hasItem(nv));
  return pkdex->getItems()[data.iHeldItem - 1];
}





void pokemon_volatile::setNoItem(const pokemon_nonvolatile& nv)
{
  data.iHeldItem = 0; 
}
