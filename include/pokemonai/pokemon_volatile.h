
#ifndef POKEMON_VOLATILE_H
#define	POKEMON_VOLATILE_H

#include "pkai.h"

#include <stdint.h>
#include <array>

#include "action.h"
#include "move_volatile.h"
#include "nonvolatile_volatile.h"
#include "team_status.h"
#include "pokemon_nonvolatile.h"

class Item;
class Pokedex;
class PokemonBase;

/*
 * contains only the metrics of a given pokemon that may change in battle, and of that set, only those
 * that are maintained when a pokemon switches out
 */
struct PKAISHARED PokemonVolatileData {
  std::array<MoveVolatileData, 4> actions;

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

  /* when 1, the current pokemon is on the field. When 0, the pokemon is in reserve. */
  uint32_t active : 1;

  /* not currently used, value must always be 0 */
  uint32_t unused : 10;

  /* Compares values of selected pokemon. Base values are compared by
   * pointer, volatile values are compared by value DEPRECIATED, use hash instead! */
  bool operator==(const PokemonVolatileData& other) const;
  bool operator!=(const PokemonVolatileData& other) const;
};

#define POKEMON_VOLATILE_IMPL_TEMPLATE \
template<typename MoveVolatileType, typename VolatileType, typename StatusType>
#define POKEMON_VOLATILE_IMPL \
PokemonVolatileImpl<MoveVolatileType, VolatileType, StatusType>

POKEMON_VOLATILE_IMPL_TEMPLATE
class PokemonVolatileImpl: public NonvolatileVolatilePair<const PokemonNonVolatile, VolatileType> {
public:
  using base_t = NonvolatileVolatilePair<const PokemonNonVolatile, VolatileType>;
  using impl_t = POKEMON_VOLATILE_IMPL;
  using status_t = StatusType;
  using movevolatile_t = MoveVolatileType;
  using base_t::base_t;
  using base_t::data;
  using base_t::nv;

  StatusType* status_;

  PokemonVolatileImpl(
      typename base_t::nonvolatile_t& nv, typename base_t::volatile_t& data) = delete;
  PokemonVolatileImpl(
      typename base_t::nonvolatile_t& nv, 
      typename base_t::volatile_t& data,
      typename impl_t::status_t& status
  ): base_t(nv, data), status_(&status) {};

  StatusType& status() const { return *status_; }
  operator StatusType&() const { return *status_; }

  /* retrieve the base species of the current volatile pokemon */
  const PokemonBase& getBase() const { return nv().getBase(); };

  /* returns TRUE if the pokemon has more than 0 hitpoints */
  bool isAlive() const;

  movevolatile_t getMV(const Action& action) const { return getMV(action.iMove()); }
  movevolatile_t getMV(size_t index) const;

  /* True if the pokemon has at least one move they can perform */
  bool hasPP() const;

  /* return the proportion of this pokemon's HP that remains, from 0..1*/
  fpType getPercentHP() const;

  /* return the integer amount of this pokemon's HP that remains, from 0..<max hp> */
  uint32_t getHP() const;

  /* return the integer amount of this pokemon's HP that's missing */
  uint32_t getMissingHP() const;

  /* get the boost pip value for the specified stat */
  int32_t getBoost(size_t type) const;

  /* get the boosted final value for the specified stat */
  uint32_t getFV_boosted(size_t type, int32_t tempBoost = 0) const;

  /* get the boosted final value for the accuracy or evasion stat */
  fpType getAccuracy_boosted(size_t type, int32_t tempBoost = 0) const;

  uint32_t getStatusAilment() const { return data().status_nonvolatile; }

  bool hasItem() const;

  const Item& getItem() const;
};


class PKAISHARED ConstPokemonVolatile: public PokemonVolatileImpl<ConstMoveVolatile,
                                                                  const PokemonVolatileData,
                                                                  const TeamStatus> {
public:
  using impl_t::impl_t;
};


class PKAISHARED PokemonVolatile: public PokemonVolatileImpl<MoveVolatile,
                                                             PokemonVolatileData,
                                                             TeamStatus> {
public:
  using impl_t::impl_t;

  operator ConstPokemonVolatile() const { return ConstPokemonVolatile{nv(), data(), status()}; }

  /* increment target's hp by quantity. */
  void modHP(int32_t quantity);

  /* set target's hp to quantity. */
  void setHP(uint32_t amt);

  /* set target's hp to % quantity of total */
  void setPercentHP(fpType percent);
  
  /* increment target's HP by percent of total. */
  void modPercentHP(fpType percent);

  /* set the boost for the specified stat */
  void setBoost(size_t type, int32_t value);

  /* modify the boost value for the specified stat */
  bool modBoost(size_t type, int32_t amt);

  /* sets the nonvolatile status condition of a pokemon*/
  void setStatusAilment(uint32_t statusCondition);

  /* clears the nonvolatile status condition of a pokemon */
  void clearStatusAilment();

  void setNoItem(bool resetVolatile=true);

  void setItem(const Item& newItem, bool resetVolatile=true);

  /*
   * initialize an empty pokemon_volatile for combat, zeroing
   * all status conditions, increasing all PP back to max, 
   * and raising HP back to normal.
   */
  void initialize(bool isActive=false);
};


PKAISHARED std::ostream& operator <<(std::ostream& os, const ConstPokemonVolatile& pokemon);

#endif	/* POKEMON_VOLATILE_H */

