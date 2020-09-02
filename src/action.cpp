#include "../inc/action.h"

#include <cstring>
#include <functional>

#include <boost/format.hpp>
#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(Action) == sizeof(uint16_t));


bool Action::operator ==(const Action& other) const {
  return std::memcmp(this, &other, sizeof(Action)) == 0;
}


void Action::print(std::ostream& os) const {
  if (isStruggle()) {
    os << "MS";
  } else if (isMove()) {
    os << boost::format("M%d") % (iMove()+1);
  } else if (isSwitch()) {
    os << boost::format("S%D") % (iFriendly()+1);
  } else if (isWait()) {
    os << 'W';
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
  // TODO(@drendleman) - use regexes
  return is;
}


size_t std::hash<Action>::operator()(const Action& a) const {
  return std::hash<uint16_t>()(*a.data());
}