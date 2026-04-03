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
  static constexpr size_t FRIENDLY_NONE = 0;
  static constexpr size_t FRIENDLY_ANY = 1;
  static constexpr size_t FRIENDLY_0 = 2;
  static constexpr size_t FRIENDLY_1 = 3;
  static constexpr size_t FRIENDLY_2 = 4;
  static constexpr size_t FRIENDLY_3 = 5;
  static constexpr size_t FRIENDLY_4 = 6;
  static constexpr size_t FRIENDLY_5 = 7;
  static constexpr size_t FRIENDLY_ADJACENT = 8;
  static constexpr size_t FRIENDLY_ADJACENT_SELF = 9;
  static constexpr size_t FRIENDLY_ACTIVE = 10;
  static constexpr size_t FRIENDLY_ALL = 11;
  static constexpr size_t FRIENDLY_SIDE = 12;
  static constexpr size_t FRIENDLY_LAST = 13;

  static constexpr size_t HOSTILE_DEFAULT = 1;
  static constexpr size_t HOSTILE_NONE = 0;
  static constexpr size_t HOSTILE_ANY = 1;
  static constexpr size_t HOSTILE_0 = 2;
  static constexpr size_t HOSTILE_1 = 3;
  static constexpr size_t HOSTILE_2 = 4;
  static constexpr size_t HOSTILE_3 = 5;
  static constexpr size_t HOSTILE_4 = 6;
  static constexpr size_t HOSTILE_5 = 7;
  static constexpr size_t HOSTILE_ADJACENT = 8;
  static constexpr size_t HOSTILE_ACTIVE = 9;
  static constexpr size_t HOSTILE_ALL = 10;
  static constexpr size_t HOSTILE_SIDE = 11;
  static constexpr size_t HOSTILE_LAST = 12;

  static Action move(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_NONE, HOSTILE_ANY};
  }

  static Action moveAlly(size_t iMove, size_t iFriendly) {
    assert(iMove < 4);
    assert(iFriendly < 6);
    return Action{MOVE_0 + iMove, FRIENDLY_0 + iFriendly, HOSTILE_NONE};
  }

  static Action moveEnemy(size_t iMove, size_t iEnemy) {
    assert(iMove < 4);
    assert(iEnemy < 6);
    return Action{MOVE_0 + iMove, FRIENDLY_NONE, HOSTILE_0 + iEnemy};
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
    return Action{MOVE_0 + iMove, FRIENDLY_NONE, HOSTILE_ADJACENT};
  }

  static Action moveAdjacentAlly(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ADJACENT, HOSTILE_NONE};
  }

  static Action moveActive(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ACTIVE, HOSTILE_ACTIVE};
  }

  static Action moveActiveEnemy(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_NONE, HOSTILE_ACTIVE};
  }

  static Action moveActiveAlly(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ACTIVE, HOSTILE_NONE};
  }

  static Action moveAll(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ALL, HOSTILE_ALL};
  }

  static Action moveAllEnemies(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_NONE, HOSTILE_ALL};
  }

  static Action moveAllAllies(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_ALL, HOSTILE_NONE};
  }

  static Action moveSideAlly(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_SIDE, HOSTILE_NONE};
  }

  static Action moveSideEnemy(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_NONE, HOSTILE_SIDE};
  }

  static Action moveSideAll(size_t iMove) {
    assert(iMove < 4);
    return Action{MOVE_0 + iMove, FRIENDLY_SIDE, HOSTILE_SIDE};
  }

  static Action swap(size_t iPokemon) {
    assert(iPokemon < 6);
    return Action{MOVE_SWITCH, FRIENDLY_0 + iPokemon, HOSTILE_NONE};
  }
  static Action struggle() {
    return Action{MOVE_STRUGGLE, FRIENDLY_NONE, HOSTILE_ANY};
  }
  static Action wait() {
    return Action{MOVE_WAIT, FRIENDLY_NONE, HOSTILE_NONE};
  }

  Action()
      : type_(MOVE_UNDEFINED),
        friendlyTarget_(FRIENDLY_DEFAULT),
        enemyTarget_(HOSTILE_DEFAULT){};
  explicit Action(
      size_t type,
      size_t friendly=FRIENDLY_DEFAULT,
      size_t hostile=HOSTILE_DEFAULT
  ): type_(type), friendlyTarget_(friendly), enemyTarget_(hostile) {
    // both friendly and hostile cannot be ANY
    assert(friendly != FRIENDLY_ANY || hostile != HOSTILE_ANY);
    assert(type < MOVE_LAST);
    assert(friendly < FRIENDLY_LAST);
    assert(hostile < HOSTILE_LAST);
  };

  size_t type() const { return type_; }
  size_t iMove() const { return type() - MOVE_0;}

  size_t friendlyTarget() const { return friendlyTarget_; }
  size_t iFriendly() const { return friendlyTarget() - FRIENDLY_0; }

  size_t enemyTarget() const { return enemyTarget_; }
  size_t iEnemy() const { return enemyTarget() - HOSTILE_0; }

  bool targetsFriendlies() const { return friendlyTarget_ != FRIENDLY_NONE; }
  bool targetsHostiles() const { return enemyTarget_ != HOSTILE_NONE; }

  bool targetsMultipleFriendly() const {
    return friendlyTarget_ >= FRIENDLY_ADJACENT;
  }
  bool targetsMultipleHostile() const {
    return enemyTarget_ >= HOSTILE_ADJACENT;
  }
  bool targetedFriendly() const {
    return friendlyTarget_ >= FRIENDLY_0 && friendlyTarget_ <= FRIENDLY_5;
  }
  bool targetedHostile() const {
    return enemyTarget_ >= HOSTILE_0 && enemyTarget_ <= HOSTILE_5;
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
std::ostream& operator<<(std::ostream& os, const ActionVector& actionVector);
std::ostream& operator<<(std::ostream& os, const ActionMap& actionMap);
std::ostream& operator<<(
    std::ostream& os, const std::vector<ActionMap>& actionMapVector);
std::istream& operator>>(std::istream& is, Action& action);

namespace std { template<> struct hash<Action> { 
  size_t operator()(const Action& a) const;
}; };

#endif /* ACTION_H */
