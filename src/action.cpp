#include "pokemonai/action.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/static_assert.hpp>
#include <cstring>
#include <functional>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

BOOST_STATIC_ASSERT(sizeof(Action) == sizeof(uint16_t));


bool Action::operator ==(const Action& other) const {
  return std::memcmp(this, &other, sizeof(Action)) == 0;
}

void Action::print() const { print(std::cout); }

void Action::print(std::ostream& os) const {
  if (isStruggle()) {
    os << "mS";
  } else if (isMove()) {
    os << fmt::format("m{}", iMove() + 1);
    if (friendlyTarget() != FRIENDLY_NONE) {
      if (friendlyTarget() == FRIENDLY_ADJACENT) {
        os << fmt::format("-fa");
      } else if (friendlyTarget() == FRIENDLY_ALL) {
        // only print a friendly target marker if enemy target isn't also all
        // (rare)
        if (enemyTarget() != HOSTILE_ALL) { os << fmt::format("-fs"); }
      } else if (friendlyTarget() == FRIENDLY_SIDE) {
        os << fmt::format("-fg");
      } else {
        os << fmt::format("-f{}", iFriendly() + 1);
      }
    }
    if (enemyTarget() != HOSTILE_NONE) {
      if (enemyTarget() == HOSTILE_ADJACENT) {
        os << fmt::format("-a");
      } else if (enemyTarget() == HOSTILE_ALL) {
        os << fmt::format("-s");
      } else if (enemyTarget() == HOSTILE_SIDE) {
        os << fmt::format("-g");
      } else {
        os << fmt::format("-{}", iEnemy() + 1);
      }
    }
  } else if (isSwitch()) {
    os << fmt::format("s{}", iFriendly() + 1);
  } else if (isActivate()) {
    os << "a";
  } else if (isWait()) {
    os << "w";
  } else if (isUndefined()) {
    os << "??";
  } else {  // unknown move!
    os << *data();
  }
}


std::ostream& operator <<(std::ostream& os, const Action& action) {
  action.print(os);
  return os;
}


std::istream& operator >>(std::istream& is, Action& action) {
  static const std::regex moveExpr("m(\\d)(?:-([f]?)([asg\\d]+))?(?:-([asg\\d]+))?");
  static const std::regex swapExpr("s(\\d)");

  // read input into string:
  bool success = false;
  std::string input;
  is >> input;
  boost::to_lower(input);
  std::smatch match;

  if (input.substr(0, 2) == "ms") {
    action = Action::struggle();
    success = true;
  } else if (input[0] == 'm') {
    if (std::regex_match(input, match, moveExpr)) {
      size_t iMove = std::stoi(match[1].str()) - 1;
      size_t friendly = Action::FRIENDLY_NONE;
      size_t hostile = Action::HOSTILE_NONE;

      auto parseTarget = [&](const std::string& prefix, const std::string& target, size_t& friendlyOut, size_t& hostileOut) {
        if (prefix == "f") {
          if (target == "a") friendlyOut = Action::FRIENDLY_ADJACENT;
          else if (target == "s") friendlyOut = Action::FRIENDLY_ALL;
          else if (target == "g") friendlyOut = Action::FRIENDLY_SIDE;
          else friendlyOut = Action::FRIENDLY_0 + std::stoi(target) - 1;
        } else {
          if (target == "a") hostileOut = Action::HOSTILE_ADJACENT;
          else if (target == "s") hostileOut = Action::HOSTILE_ALL;
          else if (target == "g") hostileOut = Action::HOSTILE_SIDE;
          else hostileOut = Action::HOSTILE_0 + std::stoi(target) - 1;
        }
      };

      if (match[3].matched) {
        parseTarget(match[2].str(), match[3].str(), friendly, hostile);
      }
      if (match[4].matched) {
        // if there's a second target, it MUST be hostile (since we already matched first as possibly friendly)
        parseTarget("", match[4].str(), friendly, hostile);
      }

      // if neither was set, default to HOSTILE_ANY
      if (friendly == Action::FRIENDLY_NONE && hostile == Action::HOSTILE_NONE) {
        hostile = Action::HOSTILE_ANY;
      }

      action = Action(Action::MOVE_0 + iMove, friendly, hostile);
      success = true;
    }
  } else if (input[0] == 's') {
    if (std::regex_match(input, match, swapExpr)) {
      if (match.size() == 2) {
        action = Action::swap(std::stoi(match[1].str()) - 1);
        success = true;
      }
    }
  } else if (input[0] == 'w') {
    action = Action::wait();
    success = true;
  } else if (input[0] == 'a') {
    action = Action::activate();
    success = true;
  }

  if (!success) { is.setstate(std::ios::failbit); }
  return is;
}


std::ostream& operator<<(std::ostream& os, const ActionMap& actionMap) {
  size_t iAction = 0;
  os << "[";
  for (const auto& [actor, action] : actionMap) {
    if (iAction > 0) { os << ", "; }
    os << fmt::format("{}: {}", fmt::streamed(actor), fmt::streamed(action));
    iAction++;
  }
  os << "]";
  return os;
}


std::ostream& operator<<(std::ostream& os, const ActionVector& actionVector) {
  size_t iAction = 0;
  os << "[";
  for (const auto& action : actionVector) {
    if (iAction > 0) { os << ", "; }
    os << fmt::format("{}", fmt::streamed(action));
    iAction++;
  }
  os << "]";
  return os;
}


std::ostream& operator<<(
    std::ostream& os, const std::vector<ActionMap>& actionMapVector) {
  size_t iActionMap = 0;
  for (const auto& actionMap : actionMapVector) {
    if (iActionMap > 0) { os << "\n"; }
    os << fmt::format("i{}: {}", iActionMap, fmt::streamed(actionMap));
    iActionMap++;
  }
  return os;
}


std::ostream& operator<<(
    std::ostream& os, const ActorActionVector& actorActionVector) {
  size_t iActionMap = 0;
  os << "[";
  for (const auto& [actor, action] : actorActionVector) {
    if (iActionMap > 0) { os << ", "; }
    os << fmt::format("{}:{}", fmt::streamed(actor), fmt::streamed(action));
    iActionMap++;
  }
  os << "]";
  return os;
}


size_t std::hash<Action>::operator()(const Action& a) const {
  uint16_t val;
  std::memcpy(&val, &a, sizeof(uint16_t));
  return std::hash<uint16_t>()(val);
}