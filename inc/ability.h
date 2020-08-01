/* 
 * File:   ability.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:45 PM
 */

#ifndef ABILITY_H
#define ABILITY_H

#include "../inc/pkai.h"

#include <string>

#include "../inc/name.h"
#include "../inc/pluggable.h"

class PKAISHARED Ability: public Name, public Pluggable
{
public:
  static const Ability* no_ability;

  Ability();
  Ability(const Ability& source);
  ~Ability();

  friend class Pokedex;
private:

  std::string script;
};

#endif	/* ABILITY_H */

