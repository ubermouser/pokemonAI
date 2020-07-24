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

class pokedex;

class PKAISHARED type: public name 
{
public:
  static const type* no_type;

  std::vector<uint8_t> modTable; // type's values
  
  type();
  type(const type& source);
  ~type() { };

  fpType getModifier(const type& other) const;
private:

};

#endif	/* TYPE_H */

