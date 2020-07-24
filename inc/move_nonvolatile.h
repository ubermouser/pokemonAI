#ifndef MOVE_NONVOLATILE_H
#define MOVE_NONVOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <ostream>
#include <assert.h>

#include "../inc/signature.h"

class pkIO;
class move;
class pokemon_nonvolatile;

union move_volatile;

#define MOVE_NONVOLATILE_DIGESTSIZE 21

class PKAISHARED move_nonvolatile : public signature<move_nonvolatile, MOVE_NONVOLATILE_DIGESTSIZE>
{

private:
  const move* base;
  uint16_t scriptVal_a;
  uint16_t scriptVal_b;
  uint8_t PPmax;

public:
  static move_nonvolatile* mNV_struggle;

  move_nonvolatile()
    : signature<move_nonvolatile, MOVE_NONVOLATILE_DIGESTSIZE>(),
    base(NULL),
    scriptVal_a(0),
    scriptVal_b(0),
    PPmax(0)
  {
  };

  move_nonvolatile(const move_nonvolatile& orig)
    : signature<move_nonvolatile, MOVE_NONVOLATILE_DIGESTSIZE>(orig),
    base(orig.base),
    scriptVal_a(orig.scriptVal_a),
    scriptVal_b(orig.scriptVal_b),
    PPmax(orig.PPmax)
  {
  };

  move_nonvolatile& operator=(const move_nonvolatile& source)
  {
    // identity theorem - simply return what we have now if equal address
    if (this == &source) { return *this; } 

    signature<move_nonvolatile, MOVE_NONVOLATILE_DIGESTSIZE>::operator=(source);
    base = source.base;
    scriptVal_a = source.scriptVal_a;
    scriptVal_b = source.scriptVal_b;
    PPmax = source.PPmax;

    return *this;
  };

  move_nonvolatile(const move& _base, unsigned int PPmultiplier = 16);

  ~move_nonvolatile() { };
  
  bool moveExists() const
  {
    return (base!=NULL)?true:false;
  };

  const move& getBase() const
  {
    assert(moveExists());
    return *base;
  };

  void setBase(const move& _base)
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

  friend class pkIO;
  friend class move_print;

  friend union move_volatile;

public:

  void createDigest_impl(boost::array<uint8_t, MOVE_NONVOLATILE_DIGESTSIZE>& digest) const;

  void initialize(pokemon_nonvolatile& cPokemon);

  void uninitialize(pokemon_nonvolatile& cPokemon);

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
  const move_nonvolatile& cMove;
  const move_volatile& currentMove;

public:
  move_print(const move_nonvolatile& _cMove, const move_volatile& _currentMove)
    : cMove(_cMove),
    currentMove(_currentMove)
  {
  };

  friend PKAISHARED std::ostream& operator <<(std::ostream& os, const move_print& cM);
};

PKAISHARED std::ostream& operator <<(std::ostream& os, const move_print& cM);

#endif /* MOVE_NONVOLATILE_H */
