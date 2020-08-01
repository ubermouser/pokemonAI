/* 
 * File:   type.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:58 PM
 */

#ifndef TYPE_H
#define	TYPE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>

#include "name.h"

class Pokedex;

class PKAISHARED Type: public Name 
{
public:
  static const Type* no_type;

  std::vector<uint8_t> modTable; // type's values
  
  Type();
  Type(const Type& source);
  ~Type() { };

  fpType getModifier(const Type& other) const;
private:

};

#endif	/* TYPE_H */

