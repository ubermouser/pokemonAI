/* 
 * File:   pokemon_base.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:27 PM
 */

#ifndef POKEMON_BASE_H
#define	POKEMON_BASE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>
#include <boost/array.hpp>

#include "../inc/name.h"

class type;
class ability;
class pokedex;

/*
 * contains metrics of a given pokemon which do not change in combat, and
 * values that the user cannot modify
 */
class PKAISHARED pokemon_base: public name
{
public:
  static const pokemon_base* no_base;
  /*
   * pokemon's typeage, lostChild is true if either of these types don't exist
   * type (HG/SS): (NOTE - ORDER AND TYPES DEFINED BY INPUT FILE!)
   *  0: normal
   *  1: fire
   *  2: water
   *  3: electric
   *  4: grass
   *  5: ice
   *  6: fighting
   *  7: poison
   *  8: ground
   *  9: flying
   * 10: psychic
   * 11: bug
   * 12: rock
   * 13: ghost
   * 14: dragon
   * 15: dark
   * 16: steel
   */
  boost::array<const type*, 2> types;

  /*
   * the weight / heftiness of the pokemon. Used for some damage calculations
   */
  uint16_t weight;
  
  /*
   * baseStats:
   * 0: Attack
   * 1: Special Attack
   * 2: Defense
   * 3: Special Defense
   * 4: Speed
   * 5: Hit-Points
   */
  boost::array<uint8_t, 6> baseStats; // pokemon's basic stats

  /* 
   * primary and secondary ability
   * NULL if not a choice
   */
  std::vector<const ability*> abilities;

  bool lostChild;

  std::vector<size_t> movelist;

  const type& getType(size_t iType) const { return *types[iType]; };

  size_t getNumAbilities() const { return abilities.size(); }

  const ability& getAbility(size_t iAbility) const { return *abilities[iAbility]; }

};

#endif	/* POKEMON_BASE_H */

