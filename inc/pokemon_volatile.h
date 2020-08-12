
#ifndef POKEMON_VOLATILE_H
#define	POKEMON_VOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>

#include "../inc/move_volatile.h"

class Item;
class Pokedex;
class pokemon_print;
class PokemonNonVolatile;

union MoveVolatile;

/*
 * contains only the metrics of a given pokemon that may change in battle, and of that set, only those
 * that are maintained when a pokemon switches out
 */
union PKAISHARED PokemonVolatile
{
  uint64_t raw;
  struct
  {
    MoveVolatile actions[4];

    /*
     * value for status ailment. 0 is no status.
     * Only one non-volatile ailment may affect a pokemon at a 
     * given time.
     * 
     * targetAilment:
     * AIL_NV_NONE: no status effect
     * AIL_NV_BURN: Burn
     * AIL_NV_FREEZE: Freeze
     * AIL_NV_PARALYSIS: Paralysis
     * AIL_NV_POISON: Poison
     * AIL_NV_TOXICPOISON: Toxic poison (number of tiers is volatile)
     * AIL_NV_SLEEP: Sleep
     * AIL_NV_REST: Like sleep, but occurs for a fixed number of turns.
     */
    uint32_t status_nonvolatile : 4;

    /* How many hitpoints the pokemon has at current. Maximum 1024 hitpoints. 0 implies a dead pokemon */
    uint32_t HPcurrent : 10;

    /* index to the item that the pokemon is holding. If 0, no item is held. If greater than 0, index is iHeldItem - 1*/
    uint32_t iHeldItem : 7;

    /* not currently used, value must always be 0 */
    uint32_t unused : 11;
  } data;

  /* Compares values of selected pokemon. Base values are compared by
   * pointer, volatile values are compared by value DEPRECIATED, use hash instead! */
  bool operator==(const PokemonVolatile& other) const;
  bool operator!=(const PokemonVolatile& other) const;
  
  const MoveVolatile& getMV(size_t index) const;

  MoveVolatile& getMV(size_t index);

  /* increment target's hp by quantity. */
  void modHP(const PokemonNonVolatile& nonvolatile, int32_t quantity);

  /* set target's hp to quantity. */
  void setHP(const PokemonNonVolatile& nonvolatile, uint32_t amt);

  /* set target's hp to % quantity of total */
  void setPercentHP(const PokemonNonVolatile& nonvolatile, fpType percent);
  
  /* increment target's HP by percent of total. */
  void modPercentHP(const PokemonNonVolatile& nonvolatile, fpType percent);
  
  /* return the proportion of this pokemon's HP that remains, from 0..1*/
  fpType getPercentHP(const PokemonNonVolatile& nonvolatile) const;

  /* return the integer amount of this pokemon's HP that remains, from 0..<max hp> */
  uint32_t getHP() const;

  /* returns TRUE if the pokemon has more than 0 hitpoints */
  bool isAlive() const;

  uint32_t getStatusAilment() const { return data.status_nonvolatile; }

  /* sets the nonvolatile status condition of a pokemon*/
  void setStatusAilment(uint32_t statusCondition);

  /* clears the nonvolatile status condition of a pokemon */
  void clearStatusAilment();

  bool hasItem(const PokemonNonVolatile& nv) const; 

  const Item& getItem(const PokemonNonVolatile& nv) const;

  void setNoItem(const PokemonNonVolatile& nv);

  /*
   * initialize an empty pokemon_volatile for combat, zeroing
   * all status conditions, increasing all PP back to max, 
   * and raising HP back to normal.
   */
  void initialize(const PokemonNonVolatile& nonvolatile);
};

#endif	/* POKEMON_VOLATILE_H */

