
//#define PKAI_EXPORT
#include "../inc/item.h"
//#undef PKAI_EXPORT

#include "../inc/type.h"

const Item* Item::no_item = NULL;

Item::Item() 
  : Name(), 
  Pluggable(),
  boostedType(Type::no_type),
  naturalGift_type(Type::no_type),
  resistedType(Type::no_type),
  flingPower(0),
  naturalGift_power(0),
  lostChild(true)
{
};





Item::Item(const Item& source) 
  : Name(source), 
  Pluggable(source),
  boostedType(source.boostedType),
  naturalGift_type(source.naturalGift_type),
  resistedType(source.resistedType),
  flingPower(source.flingPower),
  naturalGift_power(source.naturalGift_power),
  lostChild(source.lostChild)
{
};

