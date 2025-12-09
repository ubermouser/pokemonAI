#ifndef MOVE_NONVOLATILE_H
#define MOVE_NONVOLATILE_H

#include "pokemonai/pkai.h"

#include <stdint.h>
#include <iosfwd>
#include <assert.h>

#include "pokemonai/signature.h"

class PkIO;
class Move;
class PokemonNonVolatile;

class MoveVolatile;

#define MOVE_NONVOLATILE_DIGESTSIZE 21

class PKAISHARED MoveNonVolatile : public Signature<MoveNonVolatile, MOVE_NONVOLATILE_DIGESTSIZE>
{
public:
  const Move* base;
  uint16_t scriptVal_a;
  uint16_t scriptVal_b;
  uint8_t PPmax;

  static MoveNonVolatile* mNV_struggle;

  MoveNonVolatile(const Move& _base, unsigned int PPmultiplier = 16);
  MoveNonVolatile() = default;
  MoveNonVolatile(const MoveNonVolatile& orig) = default;
  MoveNonVolatile& operator=(const MoveNonVolatile& source) = default;
  virtual ~MoveNonVolatile() = default;
  
  bool moveExists() const
  {
    return (base!=NULL)?true:false;
  };

  const Move& getBase() const
  {
    assert(moveExists());
    return *base;
  };

  void setBase(const Move& _base)
  {
    base = &_base;
  };

  uint32_t getPPMax() const
  {
    return PPmax;
  };
  
  uint16_t getScriptVal_a() const
  {
    return scriptVal_a;
  };

  uint16_t getScriptVal_b() const
  {
    return scriptVal_b;
  };

  void createDigest_impl(std::array<uint8_t, MOVE_NONVOLATILE_DIGESTSIZE>& digest) const;

  void initialize(PokemonNonVolatile& cPokemon);

  void uninitialize(PokemonNonVolatile& cPokemon);

  void setScriptVal_a(uint16_t newVal)
  {
    scriptVal_a = newVal;
  };

  void setScriptVal_b(uint16_t newVal)
  {
    scriptVal_b = newVal;
  };

};

#endif /* MOVE_NONVOLATILE_H */
