
//#define PKAI_EXPORT
#include "../inc/ability.h"
//#undef PKAI_EXPORT

const ability* ability::no_ability = NULL;

ability::ability() 
	: name(), 
	pluggable(),
	script()
{
}





ability::ability(const ability& source) 
	: name(source), 
	pluggable(source),
	script(source.script)
{
}





ability::~ability() 
{
	script.clear();
}

