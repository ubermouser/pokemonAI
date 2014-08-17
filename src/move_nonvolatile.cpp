
//#define PKAI_EXPORT
#include "../inc/move_nonvolatile.h"

//#define PKAI_STATIC
#include "../inc/move.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/move_volatile.h"
//#undef PKAI_STATIC

move_nonvolatile* move_nonvolatile::mNV_struggle = NULL;

move_nonvolatile::move_nonvolatile(const move& _base, unsigned int PPmultiplier)
	: signature<move_nonvolatile, MOVE_NONVOLATILE_DIGESTSIZE>(),
	base(&_base),
	scriptVal_a(0),
	scriptVal_b(0),
	PPmax(moveExists() ? ((_base.PP * PPmultiplier) / 10) : 0)
{
};





void move_nonvolatile::initialize(pokemon_nonvolatile& cPokemon)
{
	onInitMove_rawType cPlugin = (onInitMove_rawType)getBase().getFunction(PLUGIN_ON_INIT);
	if (cPlugin == NULL) { return; }

	// call plugin:
	cPlugin(cPokemon, *this);
};





void move_nonvolatile::uninitialize(pokemon_nonvolatile& cPokemon)
{
	//int cScript = getBase().scripts[SCRIPT_ON_UNINIT];

	// test if script exists - if not, do not execute
	//if (cScript == SCRIPT_NONE) { return; }

	//TODO: call script

	// no result returned
};





void move_nonvolatile::createDigest_impl(boost::array<uint8_t, MOVE_NONVOLATILE_DIGESTSIZE>& digest) const
{
	digest.assign(0);

	size_t iDigest = 0;
	// pack first 20 characters of name:
	getBase().getName().copy((char *)(digest.c_array() + iDigest), 20, 0);
	iDigest += 20;
	/*// pack scriptvals:
	pack(scriptVal_a, digest, iDigest);
	pack(scriptVal_b, digest, iDigest);*/
	// pack PPmax:
	pack(PPmax, digest, iDigest);

	assert(iDigest == MOVE_NONVOLATILE_DIGESTSIZE);
}





std::ostream& operator <<(std::ostream& os, const move_print& cM)
{
	os << "\"" << cM.cMove.getBase().getName() << "\" " << (unsigned int)cM.currentMove.getPP() << "/" << cM.cMove.getPPMax();
	
	return os;
}
