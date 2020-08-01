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
#include <vector>

#include "../inc/name.h"
#include "../inc/pluggable.h"

class Type;
class Types;

class PKAISHARED Item : public Name, public Pluggable
{
public:
  static const Item* no_item;

  bool isImplemented() const
  {
    return Pluggable::isImplemented() && !lostChild;
  };

  const Type& getBoostedType() const
  {
    return *boostedType;
  };

  const Type& getResistedType() const
  {
    return *resistedType;
  };

  const Type& getNaturalGiftType() const
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

  Item();
  Item(const Item& source);
  ~Item() { };
  
  /* the type of move whose power is boosted by 20% when this object is held */
  const Type* boostedType;

  /* type of natural gift when used with this item */
  const Type* naturalGift_type;

  /* the type of move whose super effective damage is reduced by 50% when this object is held */
  const Type* resistedType;

  /* amount of damage dealt when fling is used with this object held */
  uint8_t flingPower;

  /* basepower of natural gift when used with this item */
  uint8_t naturalGift_power;

  bool lostChild;
};


class PKAISHARED Items: public std::vector<Item>
{
public:
  bool initialize(const std::string& path, const Types& types);

protected:
  bool loadFromFile(const std::string& path, const Types& types);
  bool loadFromFile_lines(const Types& types, const std::vector<std::string>& lines, size_t& iLine);
};

#endif	/* ITEM_H */

