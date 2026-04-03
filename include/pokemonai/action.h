/* 
 * File:   action.h
 * Author: drendleman
 *
 * Created on August 31, 2020, 4:51 PM
 */

#ifndef ACTION_H
#define ACTION_H

#include <assert.h>
#include <stdint.h>

#include <array>
#include <iosfwd>
#include <unordered_map>
#include <vector>

#include "actor.h"

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

  /**
   * FRIENDLY/HOSTILE_x - refers to a specific pokemon on the team
   * FRIENDLY/HOSTILE_DEFAULT - refers to self / the directly opposite enemy
   * FRIENDLY/HOSTILE_ADJACENT - refers to all adjacent targets
   * FRIENDLY/HOSTILE_ACTIVE - refers to all active targets
   * FRIENDLY/HOSTILE_ALL - refers to all targets
   */
  static constexpr size_t FRIENDLY_DEFAULT = 0;
  static constexpr size_t FRIENDLY_ANY = 0;
  static constexpr size_t FRIENDLY_0 = 1;
  static constexpr size_t FRIENDLY_1 = 2;
  static constexpr size_t FRIENDLY_2 = 3;
  static constexpr size_t FRIENDLY_3 = 4;
  static constexpr size_t FRIENDLY_4 = 5;
  static constexpr size_t FRIENDLY_5 = 6;
  static constexpr size_t FRIENDLY_ADJACENT = 7;
  static constexpr size_t FRIENDLY_ADJACENT_SELF = 8;
  static constexpr size_t FRIENDLY_ACTIVE = 9;
  static constexpr size_t FRIENDLY_ALL = 10;
  static constexpr size_t FRIENDLY_LAST = 11;

  static constexpr size_t HOSTILE_DEFAULT = 0;
  static constexpr size_t HOSTILE_ANY = 0;
  static constexpr size_t HOSTILE_0 = 1;
  static constexpr size_t HOSTILE_1 = 2;
  static constexpr size_t HOSTILE_2 = 3;
  static constexpr size_t HOSTILE_3 = 4;
  static constexpr size_t HOSTILE_4 = 5;
  static constexpr size_t HOSTILE_5 = 6;
  static constexpr size_t HOSTILE_ADJACENT = 7;
  static constexpr size_t HOSTILE_ACTIVE = 8;
  static constexpr size_t HOSTILE_ALL = 9;
  static constexpr size_t HOSTILE_LAST = 10;

  static Action move(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove};
  }

  static Action moveAlly(size_t iMove, size_t iFriendly) {
    assert(iMove < 4);
    assert(iFriendly < 6);
    return Action{MOVE_0 + iMove, FRIENDLY_0 + iFriendly};
  }

  static Action moveEnemy(size_t iMove, size_t iEnemy) {
    assert(iMove < 4);
    assert(iEnemy < 6);
    return Action{MOVE_0 + iMove, 0, HOSTILE_0 + iEnemy};
  }

  static Action moveTarget(
      size_t iMove, const Actor& actor, const Actor& target) {
    if (target.iTeam() == actor.iTeam()) {
      return Action::moveAlly(iMove, target.iTeammate());
    } else {
      return Action::moveEnemy(iMove, target.iTeammate());
    }
  }

  static Action moveAdjacent(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ADJACENT, HOSTILE_ADJACENT};
  }

  static Action moveAdjacentEnemy(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, 0, HOSTILE_ADJACENT};
  }

  static Action moveAdjacentAlly(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ADJACENT, 0};
  }

  static Action moveActive(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ACTIVE, HOSTILE_ACTIVE};
  }

  static Action moveActiveEnemy(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, 0, HOSTILE_ACTIVE};
  }

  static Action moveActiveAlly(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ACTIVE, 0};
  }

  static Action moveAll(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ALL, HOSTILE_ALL};
  }

  static Action moveAllEnemies(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, 0, HOSTILE_ALL};
  }

  static Action moveAllAllies(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ALL, 0};
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

  bool targetsMultipleFriendly() const {
    return friendlyTarget_ >= FRIENDLY_ADJACENT;
  }
  bool targetsMultipleHostile() const {
    return enemyTarget_ >= HOSTILE_ADJACENT;
  }

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

using ActionMap = std::unordered_map<Actor, Action>;
using ActionVector = std::vector<Action>;
using ActionPairVector = std::vector<std::array<Action, 2> >;

std::ostream& operator<<(std::ostream& os, const Action& action);
std::ostream& operator<<(std::ostream& os, const ActionMap& actionMap);
std::ostream& operator<<(
    std::ostream& os, const std::vector<ActionMap>& actionMapVector);
std::istream& operator>>(std::istream& is, Action& action);

namespace std { template<> struct hash<Action> { 
  size_t operator()(const Action& a) const;
}; };

#endif /* ACTION_H */
