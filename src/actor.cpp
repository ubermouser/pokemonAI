#include "pokemonai/actor.h"

#include <fmt/format.h>

#include <cstddef>
#include <cstring>
#include <iostream>


static_assert(sizeof(Actor) == sizeof(size_t), "Actor and size_t must be the same size");


bool Actor::operator==(const Actor& other) const {
  return std::memcmp(this, &other, sizeof(Actor)) == 0;
}


void Actor::print() const { print(std::cout); }


void Actor::print(std::ostream& os) const {
  os << fmt::format("T{}:{}", 'A' + iTeam(), iTeammate());
}


std::ostream& operator<<(std::ostream& os, const Actor& actor) {
  actor.print(os);
  return os;
}


size_t std::hash<Actor>::operator()(const Actor& a) const {
  uint64_t val;
  std::memcpy(&val, &a, sizeof(uint64_t));
  return std::hash<uint64_t>()(val);
}