/* 
 * File:   action.h
 * Author: drendleman
 *
 * Created on August 31, 2020, 4:51 PM
 */

#ifndef ACTION_H
#define ACTION_H

#include <array>
#include <assert.h>
#include <iosfwd>
#include <stdint.h>
#include <vector>

class Action {
public:
  static constexpr size_t MOVE_UNDEFINED = 0;
  static constexpr size_t MOVE_0 = 1;
  static constexpr size_t MOVE_1 = 2;
  static constexpr size_t MOVE_2 = 3;
  static constexpr size_t MOVE_3 = 4;
  static constexpr size_t MOVE_STRUGGLE = 5;
  static constexpr size_t MOVE_WAIT = 6;
  static constexpr size_t MOVE_SWITCH = 7;
  static constexpr size_t MOVE_LAST = 8;

  static constexpr size_t FRIENDLY_DEFAULT = 0;
  static constexpr size_t FRIENDLY_ANY = 0;
  static constexpr size_t FRIENDLY_0 = 1;
  static constexpr size_t FRIENDLY_1 = 2;
  static constexpr size_t FRIENDLY_2 = 3;
  static constexpr size_t FRIENDLY_3 = 4;
  static constexpr size_t FRIENDLY_4 = 5;
  static constexpr size_t FRIENDLY_5 = 6;
  static constexpr size_t FRIENDLY_ALL = 7;
  static constexpr size_t FRIENDLY_LAST = 8;

  static constexpr size_t HOSTILE_DEFAULT = 0;
  static constexpr size_t HOSTILE_ANY = 0;
  static constexpr size_t HOSTILE_ADJACENT_C = 1;
  static constexpr size_t HOSTILE_ADJACENT_L = 2;
  static constexpr size_t HOSTILE_ADJACENT_R = 3;
  static constexpr size_t HOSTILE_ALL = 4;
  static constexpr size_t HOSTILE_LAST = 5;

  static Action move(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove};
  }
  static Action moveAlly(size_t iMove, size_t iFriendly) {
    assert(iMove < 4);
    assert(iFriendly < 6);
    return Action{MOVE_0 + iMove, FRIENDLY_0 + iFriendly};
  }
  static Action swap(size_t iPokemon) {
    assert(iPokemon < 6);
    return Action{MOVE_SWITCH, FRIENDLY_0 + iPokemon};
  }
  static Action struggle() { return Action{MOVE_STRUGGLE}; }
  static Action wait() { return Action{MOVE_WAIT}; }

  Action() : type_(MOVE_UNDEFINED), friendlyTarget_(FRIENDLY_DEFAULT), enemyTarget_(HOSTILE_DEFAULT) {};
  explicit Action(
      size_t type,
      size_t friendly=FRIENDLY_DEFAULT,
      size_t hostile=HOSTILE_DEFAULT
  ): type_(type), friendlyTarget_(friendly), enemyTarget_(hostile) {
    assert(type < MOVE_LAST);
    assert(friendly < FRIENDLY_LAST);
    assert(hostile < HOSTILE_LAST);
  };

  size_t type() const { return type_; }
  size_t iMove() const { return type() - MOVE_0;}

  size_t friendlyTarget() const { return friendlyTarget_; }
  size_t iFriendly() const { return friendlyTarget() - FRIENDLY_0; }

  size_t enemyTarget() const { return enemyTarget_; }

  bool isSwitch() const { return type() == MOVE_SWITCH; }
  bool isMove() const { return type() >= MOVE_0 && type_ <= MOVE_STRUGGLE; }
  bool isStruggle() const { return type() == MOVE_STRUGGLE; }
  bool isWait() const { return type() == MOVE_WAIT; }
  bool isUndefined() const { return type() == MOVE_UNDEFINED; }

  const uint16_t* data() const { return reinterpret_cast<const uint16_t*>(this); }
  uint16_t* data() { return reinterpret_cast<uint16_t*>(this); }

  bool operator ==(const Action& other) const;
  bool operator !=(const Action& other) const { return !(*this == other); }

  void print() const;
  void print(std::ostream& os) const;

protected:
  uint16_t type_: 6;

  uint16_t friendlyTarget_: 5;

  uint16_t enemyTarget_: 5;

};

using ActionVector = std::vector<Action>;
using ActionPairVector = std::vector<std::array<Action, 2> >;

std::ostream& operator <<(std::ostream& os, const Action& action);
std::istream& operator >>(std::istream& is, Action& action);

namespace std { template<> struct hash<Action> { 
  size_t operator()(const Action& a) const;
}; };

#endif /* ACTION_H */
