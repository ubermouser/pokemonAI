/* 
 * File:   item.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:37 PM
 */

#ifndef ITEM_H
#define	ITEM_H

#include "../inc/pkai.h"

#include <string>

#include "../inc/name.h"
#include "../inc/pluggable.h"

class type;

class PKAISHARED item : public name, public pluggable
{
public:
  static const item* no_item;

  bool isImplemented() const
  {
    return pluggable::isImplemented() && !lostChild;
  };

  const type& getBoostedType() const
  {
    return *boostedType;
  };

  const type& getResistedType() const
  {
    return *resistedType;
  };

  const type& getNaturalGiftType() const
  {
    return *naturalGift_type;
  };

  uint8_t getNaturalGiftPower() const
  {
    return naturalGift_power;
  };

  uint8_t getFlingPower() const
  {
    return flingPower;
  };

  item();
  item(const item& source);
  ~item() { };
  
private:
  /* the type of move whose power is boosted by 20% when this object is held */
  const type* boostedType;

  /* type of natural gift when used with this item */
  const type* naturalGift_type;

  /* the type of move whose super effective damage is reduced by 50% when this object is held */
  const type* resistedType;

  /* amount of damage dealt when fling is used with this object held */
  uint8_t flingPower;

  /* basepower of natural gift when used with this item */
  uint8_t naturalGift_power;

  bool lostChild;

  friend class pokedex;
};

#endif	/* ITEM_H */

