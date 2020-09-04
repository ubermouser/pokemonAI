
//#define PKAI_EXPORT
#include "../inc/move_nonvolatile.h"

//#define PKAI_STATIC
#include "../inc/move.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/move_volatile.h"
#include "../inc/pluggable_types.h"
//#undef PKAI_STATIC

MoveNonVolatile* MoveNonVolatile::mNV_struggle = NULL;

MoveNonVolatile::MoveNonVolatile(const Move& _base, unsigned int PPmultiplier)
  : Signature<MoveNonVolatile, MOVE_NONVOLATILE_DIGESTSIZE>(),
  base(&_base),
  scriptVal_a(0),
  scriptVal_b(0),
  PPmax(moveExists() ? ((_base.PP_ * PPmultiplier) / 10) : 0)
{
};





void MoveNonVolatile::initialize(PokemonNonVolatile& cPokemon)
{
  onInitMove_rawType cPlugin = (onInitMove_rawType)getBase().getFunction(PLUGIN_ON_INIT);
  if (cPlugin == NULL) { return; }

  // call plugin:
  cPlugin(cPokemon, *this);
};





void MoveNonVolatile::uninitialize(PokemonNonVolatile& cPokemon)
{
  //int cScript = getBase().scripts[SCRIPT_ON_UNINIT];

  // test if script exists - if not, do not execute
  //if (cScript == SCRIPT_NONE) { return; }

  //TODO: call script

  // no result returned
};





void MoveNonVolatile::createDigest_impl(std::array<uint8_t, MOVE_NONVOLATILE_DIGESTSIZE>& digest) const
{
  digest.fill(0);

  size_t iDigest = 0;
  // pack first 20 characters of name:
  getBase().getName().copy((char *)(digest.data() + iDigest), 20, 0);
  iDigest += 20;
  /*// pack scriptvals:
  pack(scriptVal_a, digest, iDigest);
  pack(scriptVal_b, digest, iDigest);*/
  // pack PPmax:
  pack(PPmax, digest, iDigest);

  assert(iDigest == MOVE_NONVOLATILE_DIGESTSIZE);
}

