#ifndef MOVE_NONVOLATILE_H
#define MOVE_NONVOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <ostream>
#include <assert.h>

#include "../inc/signature.h"

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

  MoveNonVolatile()
    : Signature<MoveNonVolatile, MOVE_NONVOLATILE_DIGESTSIZE>(),
    base(NULL),
    scriptVal_a(0),
    scriptVal_b(0),
    PPmax(0)
  {
  };

  MoveNonVolatile(const MoveNonVolatile& orig)
    : Signature<MoveNonVolatile, MOVE_NONVOLATILE_DIGESTSIZE>(orig),
    base(orig.base),
    scriptVal_a(orig.scriptVal_a),
    scriptVal_b(orig.scriptVal_b),
    PPmax(orig.PPmax)
  {
  };

  MoveNonVolatile& operator=(const MoveNonVolatile& source)
  {
    // identity theorem - simply return what we have now if equal address
    if (this == &source) { return *this; } 

    Signature<MoveNonVolatile, MOVE_NONVOLATILE_DIGESTSIZE>::operator=(source);
    base = source.base;
    scriptVal_a = source.scriptVal_a;
    scriptVal_b = source.scriptVal_b;
    PPmax = source.PPmax;

    return *this;
  };

  MoveNonVolatile(const Move& _base, unsigned int PPmultiplier = 16);

  ~MoveNonVolatile() { };
  
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

class PKAISHARED move_print
{
private:
  const MoveNonVolatile& cMove;
  const MoveVolatile& currentMove;

public:
  move_print(const MoveNonVolatile& _cMove, const MoveVolatile& _currentMove)
    : cMove(_cMove),
    currentMove(_currentMove)
  {
  };

  friend PKAISHARED std::ostream& operator <<(std::ostream& os, const move_print& cM);
};

PKAISHARED std::ostream& operator <<(std::ostream& os, const move_print& cM);

#endif /* MOVE_NONVOLATILE_H */
