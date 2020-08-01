
//#define PKAI_EXPORT
#include "../inc/type.h"
//#undef PKAI_EXPORT

//#define PKAI_STATIC
#include "../inc/pokedex.h"
//#undef PKAI_STATIC

const Type* Type::no_type = NULL;



Type::Type() 
  : modTable()
{
}





Type::Type(const Type& source) 
  : modTable(source.modTable)
{
}





fpType Type::getModifier(const Type& other) const
{
  size_t iOType = &other - &pkdex->getTypes().front();

  return (fpType)modTable[iOType] / (fpType) FPMULTIPLIER;
}