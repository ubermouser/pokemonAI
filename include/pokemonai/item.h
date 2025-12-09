/* 
 * File:   item.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:37 PM
 */

#ifndef ITEM_H
#define	ITEM_H

#include "pokemonai/pkai.h"

#include <string>
#include <vector>

#include "pokemonai/name.h"
#include "pokemonai/collection.h"
#include "pokemonai/pluggable.h"

class Type;
class Types;

class PKAISHARED Item : public Name, public Pluggable
{
public:
  static const Item* no_item;

  bool isImplemented() const override
  {
    return Pluggable::isImplemented() && !lostChild_;
  };

  const Type& getBoostedType() const
  {
    return *boostedType_;
  };

  const Type& getResistedType() const
  {
    return *resistedType_;
  };

  const Type& getNaturalGiftType() const
  {
    return *naturalGiftType_;
  };

  uint8_t getNaturalGiftPower() const
  {
    return naturalGiftPower_;
  };

  uint8_t getFlingPower() const
  {
    return flingPower_;
  };

  Item();
  Item(const Item& source) = default;
  virtual ~Item() override = default;
  
  /* the type of move whose power is boosted by 20% when this object is held */
  const Type* boostedType_;

  /* type of natural gift when used with this item */
  const Type* naturalGiftType_;

  /* the type of move whose super effective damage is reduced by 50% when this object is held */
  const Type* resistedType_;

  /* TODO: delete. The index of insertion of this item. */
  size_t index_ = MAXITEMS;

  /* amount of damage dealt when fling is used with this object held */
  uint8_t flingPower_;

  /* basepower of natural gift when used with this item */
  uint8_t naturalGiftPower_;

  bool lostChild_ = true;
};


class PKAISHARED Items: public Collection<Item>
{
public:
  bool initialize(const std::string& path, const Types& types);

  const Item* atByIndex(size_t index) const { return byIndex_.at(index); }

protected:
  bool loadFromFile(const std::string& path, const Types& types);
  bool loadFromFile_lines(const Types& types, const std::vector<std::string>& lines, size_t& iLine);

  std::unordered_map<size_t, const Item*> byIndex_;
};

#endif	/* ITEM_H */

