//#define PKAI_EXPORT
#include "../inc/move.h"
//#undef PKAI_EXPORT

#include "../inc/init_toolbox.h"

//#define PKAI_STATIC
#include "../inc/move_volatile.h"
#include "../inc/type.h"
#include "../inc/pokedex.h"
//#undef PKAI_STATIC

const move* move::move_struggle = NULL;
const move* move::move_none = NULL;

void move::init(const move& source)
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





move::move()
	: name(),
	pluggable(),
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
	selfBuff.assign(0);
	targetDebuff.assign(0);
	
	targetAilment = AIL_NV_NONE;
	targetVolatileAilment = AIL_V_NONE;
	
	lostChild = true;
}





move::move(const move& source)
	: name(source),
	pluggable(source)
{
	init(source);
}





move& move::operator=(const move& source)
{
	// identity theorem - simply return what we have now if equal address
	if (this == &source) { return *this; } 
	
	name::operator=(source);
	pluggable::operator=(source);
	
	init(source);
	
	return *this;
}





move::~move()
{
	description.clear();
}





const type& move::getType() const
{
	assert(cType != NULL);
	return *cType;
}
