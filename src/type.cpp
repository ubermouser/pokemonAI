
//#define PKAI_EXPORT
#include "../inc/type.h"
//#undef PKAI_EXPORT

//#define PKAI_STATIC
#include "../inc/pokedex.h"
//#undef PKAI_STATIC

const type* type::no_type = NULL;



type::type() 
	: modTable()
{
}





type::type(const type& source) 
	: modTable(source.modTable)
{
}





fpType type::getModifier(const type& other) const
{
	size_t iOType = &other - &pkdex->getTypes().front();

	return (fpType)modTable[iOType] / (fpType) FPMULTIPLIER;
}