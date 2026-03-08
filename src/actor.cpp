#include "pokemonai/actor.h"

#include <cstddef>
#include <cstring>


static_assert(sizeof(Actor) == sizeof(size_t), "Actor and size_t must be the same size");


bool Actor::operator==(const Actor& other) const {
  return std::memcmp(this, &other, sizeof(Actor)) == 0;
}


size_t std::hash<Actor>::operator()(const Actor& a) const {
  uint64_t val;
  std::memcpy(&val, &a, sizeof(uint64_t));
  return std::hash<uint64_t>()(val);
}