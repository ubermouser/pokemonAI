#include "../inc/action.h"

#include <cstring>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <stdexcept>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>
#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(Action) == sizeof(uint16_t));


bool Action::operator ==(const Action& other) const {
  return std::memcmp(this, &other, sizeof(Action)) == 0;
}


void Action::print() const { std::cout << *this << "\n"; }


void Action::print(std::ostream& os) const {
  if (isStruggle()) {
    os << "mS";
  } else if (isMove()) {
    os << boost::format("m%d") % (iMove()+1);
    if (friendlyTarget() != FRIENDLY_DEFAULT) {
      os << boost::format("-%d") % (friendlyTarget());
    }
  } else if (isSwitch()) {
    os << boost::format("s%d") % (iFriendly()+1);
  } else if (isWait()) {
    os << "w";
  } else if (isUndefined()) {
    os << "??";
  } else { // unknown move!
    os << *data();
  }
}


std::ostream& operator <<(std::ostream& os, const Action& action) {
  action.print(os);
  return os;
}


std::istream& operator >>(std::istream& is, Action& action) {
  static const std::regex moveExpr("m(\\d)"); // TODO(@drendleman) old expression "m(\\d)(?:-(\\d))?"
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
      /*if (match.size() == 4) {
        action = Action::moveAlly(std::stoi(match[1].str()) - 1, std::stoi(match[3].str()) - 1);
        success = true;
      } else*/ if (match.size() == 2) {
        action = Action::move(std::stoi(match[1].str()) - 1);
        success = true;
      }
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
  }
  
  if (!success) { is.setstate(std::ios::failbit); }
  return is;
}


size_t std::hash<Action>::operator()(const Action& a) const {
  return std::hash<uint16_t>()(*a.data());
}