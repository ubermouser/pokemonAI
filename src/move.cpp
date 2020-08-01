//#define PKAI_EXPORT
#include "../inc/move.h"
//#undef PKAI_EXPORT

#include "../inc/init_toolbox.h"

//#define PKAI_STATIC
#include "../inc/move_volatile.h"
#include "../inc/type.h"
#include "../inc/pokedex.h"
//#undef PKAI_STATIC

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
