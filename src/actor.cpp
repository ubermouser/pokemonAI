#include "pokemonai/actor.h"

#include <fmt/format.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <regex>


static_assert(sizeof(Actor) == sizeof(size_t), "Actor and size_t must be the same size");


bool Actor::operator==(const Actor& other) const {
  return std::memcmp(this, &other, sizeof(Actor)) == 0;
}


void Actor::print() const { print(std::cout); }


void Actor::print(std::ostream& os) const {
  os << fmt::format("T{:c}:{}", 'A' + iTeam(), iTeammate() + 1);
}


std::ostream& operator<<(std::ostream& os, const Actor& actor) {
  actor.print(os);
  return os;
}


std::istream& operator>>(std::istream& is, Actor& actor) {
  static const std::regex actorRegex("t([ab]):?([1-6])");

  bool success = false;
  std::string input;
  is >> input;
  boost::to_lower(input);
  std::smatch match;
  if (std::regex_match(input, match, actorRegex)) {
    actor.iTeam_ = match[1].str()[0] - 'a';
    actor.iTeammate_ = std::stoi(match[2].str()) - 1;
    success = true;
  }
  if (!success) { is.setstate(std::ios::failbit); }
  return is;
}


size_t std::hash<Actor>::operator()(const Actor& a) const {
  uint64_t val = 0;
  std::memcpy(&val, &a, sizeof(Actor));
  return std::hash<uint64_t>()(val);
}