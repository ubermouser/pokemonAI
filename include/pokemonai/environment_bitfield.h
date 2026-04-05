#ifndef PKAI_ENVIRONMENT_BITFIELD_H
#define PKAI_ENVIRONMENT_BITFIELD_H

#include <stddef.h>
#include <stdint.h>

#include "actor.h"

struct ActorFlags {
  union {
    struct {
      uint8_t hit : 1;
      uint8_t crit : 1;
      uint8_t secondary : 1;
      uint8_t blocked : 1;
      uint8_t switched : 1;
      uint8_t free : 1;
      uint8_t wait : 1;
      uint8_t movesFirst : 1;
    };
    uint8_t raw;
  };

  ActorFlags() : raw(0) {}
  explicit ActorFlags(uint8_t r) : raw(r) {}
};

enum ActorFlag : uint8_t {
  FLAG_HIT         = 1 << 0,
  FLAG_CRIT        = 1 << 1,
  FLAG_SECONDARY   = 1 << 2,
  FLAG_BLOCKED     = 1 << 3,
  FLAG_SWITCHED    = 1 << 4,
  FLAG_FREE_MOVE   = 1 << 5,
  FLAG_WAITED       = 1 << 6,
  FLAG_MOVES_FIRST = 1 << 7,
};

union EnvironmentBitfield;

template <typename BitfieldType>
struct ActorProxyImpl {
  BitfieldType& b;
  size_t iTeam;
  size_t iTeammate;

  ActorProxyImpl(BitfieldType& bitfield, size_t team, size_t teammate)
      : b(bitfield), iTeam(team), iTeammate(teammate) {}

  template <typename OtherBitfieldType>
  ActorProxyImpl(const ActorProxyImpl<OtherBitfieldType>& other)
      : b(other.b), iTeam(other.iTeam), iTeammate(other.iTeammate) {}

  static constexpr size_t ALL_TEAMMATES = 6;

  // Helpers
  bool getFlag(ActorFlag flag) const;
  ActorProxyImpl& setFlag(ActorFlag flag, bool val = true);

  // Standard Getters
  bool isHit() const { return getFlag(FLAG_HIT); }
  bool isCrit() const { return getFlag(FLAG_CRIT); }
  bool isSecondary() const { return getFlag(FLAG_SECONDARY); }
  bool isBlocked() const { return getFlag(FLAG_BLOCKED); }
  bool isSwitched() const { return getFlag(FLAG_SWITCHED); }
  bool isFreeMove() const { return getFlag(FLAG_FREE_MOVE); }
  bool isWaited() const { return getFlag(FLAG_WAITED); }
  bool isMovedFirst() const { return getFlag(FLAG_MOVES_FIRST); }

  // Standard Setters (Available only for non-const BitfieldType)
  ActorProxyImpl& setHit(bool val = true) { return setFlag(FLAG_HIT, val); }
  ActorProxyImpl& setCrit(bool val = true) { return setFlag(FLAG_CRIT, val); }
  ActorProxyImpl& setSecondary(bool val = true) { return setFlag(FLAG_SECONDARY, val); }
  ActorProxyImpl& setBlocked(bool val = true) { return setFlag(FLAG_BLOCKED, val); }
  ActorProxyImpl& setSwitched(bool val = true) { return setFlag(FLAG_SWITCHED, val); }
  ActorProxyImpl& setFreeMove(bool val = true) { return setFlag(FLAG_FREE_MOVE, val); }
  ActorProxyImpl& setWaited(bool val = true) { return setFlag(FLAG_WAITED, val); }
  ActorProxyImpl& setMovedFirst(bool val = true) { return setFlag(FLAG_MOVES_FIRST, val); }



  ActorProxyImpl flagsFor(size_t nextITeam, size_t nextITeammate) {
    return ActorProxyImpl<BitfieldType>(b, nextITeam, nextITeammate);
  }
  ActorProxyImpl flagsFor(const Actor& a) {
    return ActorProxyImpl<BitfieldType>(b, a.iTeam(), a.iTeammate());
  }
  ActorProxyImpl flagsFor(size_t nextITeam, size_t nextITeammate) const {
    return ActorProxyImpl<const BitfieldType>(b, nextITeam, nextITeammate);
  }
  ActorProxyImpl flagsFor(const Actor& a) const {
    return ActorProxyImpl<const BitfieldType>(b, a.iTeam(), a.iTeammate());
  }

  ActorProxyImpl& setPruned(bool val = true);
  ActorProxyImpl& setMerged(bool val = true);


  EnvironmentBitfield collapseTeams() const;

  operator EnvironmentBitfield() const;
};

using ActorProxy = ActorProxyImpl<EnvironmentBitfield>;
using ConstActorProxy = ActorProxyImpl<const EnvironmentBitfield>;

union EnvironmentBitfield {
  struct {
    union {
      struct {
        ActorFlags team0[6];
        ActorFlags team1[6];
      };
      ActorFlags team[2][6];
    };
    uint16_t pruned : 1;
    uint16_t merged : 1;
    uint16_t _pad : 14;
    uint16_t _pad2;
  } bits;
  uint64_t raw[2];

  EnvironmentBitfield() { raw[0] = 0; raw[1] = 0; }
  EnvironmentBitfield(uint64_t r0, uint64_t r1 = 0) { raw[0] = r0; raw[1] = r1; }

  bool operator==(const EnvironmentBitfield& other) const {
    return raw[0] == other.raw[0] && raw[1] == other.raw[1];
  }
  bool operator!=(const EnvironmentBitfield& other) const {
    return !(*this == other);
  }
  EnvironmentBitfield operator&(const EnvironmentBitfield& other) const {
    return {raw[0] & other.raw[0], raw[1] & other.raw[1]};
  }
  EnvironmentBitfield operator|(const EnvironmentBitfield& other) const {
    return {raw[0] | other.raw[0], raw[1] | other.raw[1]};
  }
  EnvironmentBitfield& operator&=(const EnvironmentBitfield& other) {
    raw[0] &= other.raw[0];
    raw[1] &= other.raw[1];
    return *this;
  }
  EnvironmentBitfield& operator|=(const EnvironmentBitfield& other) {
    raw[0] |= other.raw[0];
    raw[1] |= other.raw[1];
    return *this;
  }
  EnvironmentBitfield operator~() const {
    return {~raw[0], ~raw[1]};
  }
  
  // Returns a bitfield where all teammate flags are ORed into Slot 0.
  // Useful for "any teammate" matching in masks.
  EnvironmentBitfield collapseTeams() const {
    EnvironmentBitfield result{};
    for (int i = 0; i < 6; ++i) {
      result.bits.team[0][0].raw |= bits.team[0][i].raw;
      result.bits.team[1][0].raw |= bits.team[1][i].raw;
    }
    result.bits.pruned = bits.pruned;
    result.bits.merged = bits.merged;
    return result;
  }


  ActorFlags getActorFlags(size_t iTeam, size_t iTeammate) const {
    return bits.team[iTeam][iTeammate];
  }

  void setActorFlags(size_t iTeam, size_t iTeammate, ActorFlags flags) {
    bits.team[iTeam][iTeammate] = flags;
  }

  ActorProxy flagsFor(size_t iTeam, size_t iTeammate) { return ActorProxy(*this, iTeam, iTeammate); }
  ActorProxy flagsFor(const Actor& a) { return ActorProxy(*this, a.iTeam(), a.iTeammate()); }
  ConstActorProxy flagsFor(size_t iTeam, size_t iTeammate) const { return ConstActorProxy(*this, iTeam, iTeammate); }
  ConstActorProxy flagsFor(const Actor& a) const { return ConstActorProxy(*this, a.iTeam(), a.iTeammate()); }

  uint8_t getTeamOr(size_t iTeam) const {
    uint8_t result = 0;
    for (size_t i = 0; i < 6; ++i) result |= bits.team[iTeam][i].raw;
    return result;
  }
  

  EnvironmentBitfield& setPruned(bool val = true) {
    bits.pruned = val;
    return *this;
  }

  EnvironmentBitfield& setMerged(bool val = true) {
    bits.merged = val;
    return *this;
  }

private:
  uint8_t& getRawActor(size_t iTeam, size_t iTeammate) {
    return bits.team[iTeam][iTeammate].raw;
  }

  uint8_t getRawActor(size_t iTeam, size_t iTeammate) const {
    return bits.team[iTeam][iTeammate].raw;
  }
};

// ActorProxyImpl Implementations
template <typename T>
inline bool ActorProxyImpl<T>::getFlag(ActorFlag flag) const {
  if (iTeammate == ALL_TEAMMATES) return b.getTeamOr(iTeam) & flag;
  return b.getActorFlags(iTeam, iTeammate).raw & flag;
}

template <typename T>
inline ActorProxyImpl<T>& ActorProxyImpl<T>::setFlag(ActorFlag flag, bool val) {
  if (iTeammate == ALL_TEAMMATES) {
    for (size_t i = 0; i < 6; ++i) b.flagsFor(iTeam, i).setFlag(flag, val);
    return *this;
  }
  ActorFlags f = b.getActorFlags(iTeam, iTeammate);
  if (val) f.raw |= flag;
  else f.raw &= ~flag;
  b.setActorFlags(iTeam, iTeammate, f);
  return *this;
}

template <typename T>
inline ActorProxyImpl<T>& ActorProxyImpl<T>::setPruned(bool val) {
  b.setPruned(val);
  return *this;
}

template <typename T>
inline ActorProxyImpl<T>& ActorProxyImpl<T>::setMerged(bool val) {
  b.setMerged(val);
  return *this;
}

template <typename T>
inline ActorProxyImpl<T>::operator EnvironmentBitfield() const { return b; }

template <typename T>
inline EnvironmentBitfield ActorProxyImpl<T>::collapseTeams() const { return b.collapseTeams(); }


#endif // PKAI_ENVIRONMENT_BITFIELD_H
