
//#define PKAI_EXPORT
#include "../inc/item.h"
//#undef PKAI_EXPORT

#include "../inc/type.h"

const item* item::no_item = NULL;

item::item() 
  : name(), 
  pluggable(),
  boostedType(type::no_type),
  naturalGift_type(type::no_type),
  resistedType(type::no_type),
  flingPower(0),
  naturalGift_power(0),
  lostChild(true)
{
};





item::item(const item& source) 
  : name(source), 
  pluggable(source),
  boostedType(source.boostedType),
  naturalGift_type(source.naturalGift_type),
  resistedType(source.resistedType),
  flingPower(source.flingPower),
  naturalGift_power(source.naturalGift_power),
  lostChild(source.lostChild)
{
};

