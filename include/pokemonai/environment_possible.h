/* 
 * File:   possible_environment.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:25 PM
 */

#ifndef POSSIBLE_ENVIRONMENT_H
#define	POSSIBLE_ENVIRONMENT_H

#include <stdint.h>

#include <deque>
#include <iosfwd>
#include <memory>

#include "actor.h"
#include "environment_nonvolatile.h"
#include "environment_volatile.h"
#include "fixedpoint/fixed_class.h"
#include "nonvolatile_volatile.h"
#include "pkai.h"


struct TeamEnvironmentFlags {
  uint8_t hit : 1;
  uint8_t crit : 1;
  uint8_t secondary : 1;
  uint8_t blocked : 1;
  uint8_t switched : 1;
  uint8_t free : 1;
  uint8_t wait : 1;
  uint8_t movesFirst : 1;
};

union EnvironmentBitfield;

struct TeamProxy {
  EnvironmentBitfield& b;
  size_t i;

  TeamProxy& hasHit();
  TeamProxy& hasCrit();
  TeamProxy& hasSecondary();
  TeamProxy& wasBlocked();
  TeamProxy& hasSwitched();
  TeamProxy& hasFree();
  TeamProxy& hasWait();
  TeamProxy& hasMovedFirst();

  TeamProxy team(size_t nextI) { return {b, nextI}; }

  TeamProxy& isPruned();
  TeamProxy& isMerged();

  operator EnvironmentBitfield() const;
};

union EnvironmentBitfield {
  uint32_t raw;
  struct {
    TeamEnvironmentFlags team0;
    uint8_t _pad0 : 7;
    uint8_t pruned : 1;
    TeamEnvironmentFlags team1;
    uint8_t _pad1 : 7;
    uint8_t merged : 1;
  } bits;

  EnvironmentBitfield() : raw(0) {}
  EnvironmentBitfield(uint32_t raw) : raw(raw) {}

  const TeamEnvironmentFlags& getTeam(size_t i) const {
    return (i == 0) ? bits.team0 : bits.team1;
  }

  TeamEnvironmentFlags& getTeam(size_t i) {
    return (i == 0) ? bits.team0 : bits.team1;
  }

  TeamProxy team(size_t i) { return {*this, i}; }

  EnvironmentBitfield& isPruned() {
    bits.pruned = 1;
    return *this;
  }

  EnvironmentBitfield& isMerged() {
    bits.merged = 1;
    return *this;
  }
};

inline TeamProxy& TeamProxy::hasHit() {
  b.getTeam(i).hit = 1;
  return *this;
}
inline TeamProxy& TeamProxy::hasCrit() {
  b.getTeam(i).crit = 1;
  return *this;
}
inline TeamProxy& TeamProxy::hasSecondary() {
  b.getTeam(i).secondary = 1;
  return *this;
}
inline TeamProxy& TeamProxy::wasBlocked() {
  b.getTeam(i).blocked = 1;
  return *this;
}
inline TeamProxy& TeamProxy::hasSwitched() {
  b.getTeam(i).switched = 1;
  return *this;
}
inline TeamProxy& TeamProxy::hasFree() {
  b.getTeam(i).free = 1;
  return *this;
}
inline TeamProxy& TeamProxy::hasWait() {
  b.getTeam(i).wait = 1;
  return *this;
}
inline TeamProxy& TeamProxy::hasMovedFirst() {
  b.getTeam(i).movesFirst = 1;
  return *this;
}
inline TeamProxy& TeamProxy::isPruned() {
  b.bits.pruned = 1;
  return *this;
}
inline TeamProxy& TeamProxy::isMerged() {
  b.bits.merged = 1;
  return *this;
}
inline TeamProxy::operator EnvironmentBitfield() const { return b; }


struct PKAISHARED EnvironmentPossibleData {
  /*
   * the environment this possible_environment represents
   */
  EnvironmentVolatileData env;

  /* the hashed value of this completed environment_possible. Environment should NOT BE CHANGED once hashed! */
  uint64_t hash;

  /*
   * the probability that this environment will occur 
   * given the previous environment's probability is 1, and
   * the probability of the actions that occured to create this
   * environment are 1
   */
  FixType probability;

  /*
   * what type of action occured to create this environment from
   * the previous environment?
   * 
   * team 0 (bits 0-7) / team 1 (bits 16-23):
   * hit
   * crit
   * secondary
   * blocked
   * switched
   * free
   * wait
   * movesFirst
   * 
   * global:
   * pruned (bit 15)
   * merged (bit 31)
   */
  EnvironmentBitfield flags;


  static EnvironmentPossibleData create(const EnvironmentVolatileData& source, bool doHash = true);

  /* Is the probability of this entity occuring less than the probability of
   the other entity occuring?*/
  bool operator<(const EnvironmentPossibleData& other) const;

  uint64_t generateHash();

  const uint64_t& getHash() const { return hash; };

  const FixType& getProbability() const { return probability; };
  FixType& getProbability() { return probability; };

  const uint32_t& getBitmask() const { return flags.raw; };
  uint32_t& getBitmask() { return flags.raw; };

  void setMerged() {
    flags.bits.merged = 1;
  };

  void setPruned() {
    flags.bits.pruned = 1;
  };
  

  bool isPruned() const {
    return flags.bits.pruned;
  };

  bool isMerged() const {
    return flags.bits.merged;
  }
};


#define ENV_POSSIBLE_IMPL_TEMPLATE template<typename EnvVolatileType, typename VolatileType>
#define ENV_POSSIBLE_IMPL EnvironmentPossibleImpl<EnvVolatileType, VolatileType>


ENV_POSSIBLE_IMPL_TEMPLATE
class PKAISHARED EnvironmentPossibleImpl: public NonvolatileVolatilePair<const EnvironmentNonvolatile, VolatileType> {
public:
  using base_t = NonvolatileVolatilePair<const EnvironmentNonvolatile, VolatileType>;
  using impl_t = ENV_POSSIBLE_IMPL;
  using environmentvolatile_t = EnvVolatileType;
  using base_t::base_t;
  using base_t::data;
  using base_t::nv;
  using team_t = typename environmentvolatile_t::teamvolatile_t;
  using pokemon_t = typename team_t::pokemonvolatile_t;

  /* print details of a single state */
  void printState() const;
  void printState(std::ostream& os) const;
  void printEnvironment(std::ostream& os) const;

  operator environmentvolatile_t() const { return environmentvolatile_t{nv(), data().env}; };
  environmentvolatile_t getEnv() const { return environmentvolatile_t{nv(), data().env}; };

  const FixType& getProbability() const { return data().getProbability(); };

  const uint64_t& getHash() const { return data().getHash(); };

  const uint32_t& getBitmask() const { return data().getBitmask(); };

  /* has iTeam hit this round? */
  bool hasHit(size_t iTeam) const {
    return data().flags.getTeam(iTeam).hit;
  };

  /* has iTeam critical hit this round? */
  bool hasCrit(size_t iTeam) const {
    return data().flags.getTeam(iTeam).crit;
  };

  /* has iTeam used a secondary effect this round? */
  bool hasSecondary(size_t iTeam) const {
    return data().flags.getTeam(iTeam).secondary;
  };

  /* was iteam's action blocked this round? */
  bool wasBlocked(size_t iTeam) const {
    return data().flags.getTeam(iTeam).blocked;
  };

  /* has iTeam switched this round? */
  bool hasSwitched(size_t iTeam) const {
    return data().flags.getTeam(iTeam).switched;
  };

  /* has iTeam used a free move this round? */
  bool hasFreeMove(size_t iTeam) const {
    return data().flags.getTeam(iTeam).free;
  };

  /* has iTeam used waited this round? */
  bool hasWaited(size_t iTeam) const {
    return data().flags.getTeam(iTeam).wait;
  };

  /* has iTeam moved first this round? */
  bool hasMovedFirst(size_t iTeam) const {
    return data().flags.getTeam(iTeam).movesFirst;
  };

  bool isMerged() const { return data().isMerged(); }
  bool isPruned() const { return data().isPruned(); }

  bool isEmpty() const;

  team_t getTeam(size_t iTeam) const { return getEnv().getTeam(iTeam); }

  pokemon_t teammate(const Actor& actor) const {
    return getEnv().teammate(actor);
  }
  pokemon_t teammate(size_t iTeam, size_t iTeammate) const {
    return getEnv().teammate(iTeam, iTeammate);
  }
};


class PKAISHARED ConstEnvironmentPossible: public EnvironmentPossibleImpl<ConstEnvironmentVolatile, const EnvironmentPossibleData> {
public:
  using impl_t::impl_t;

  // TODO(@drendleman) a workaround for stateSelect having to return a NULL State
  explicit ConstEnvironmentPossible(nonvolatile_t& nv);
};


class PKAISHARED EnvironmentPossible: public EnvironmentPossibleImpl<EnvironmentVolatile, EnvironmentPossibleData> {
public:
  using impl_t::impl_t;

  operator ConstEnvironmentPossible() const { return ConstEnvironmentPossible{nv(), data()}; };

  uint32_t& getBitmask() { return data().getBitmask(); };

  FixType& getProbability() { return data().getProbability(); };
  const FixType& getProbability() const { return data().getProbability(); };

  void setHit(size_t iTeam) {
    data().flags.getTeam(iTeam).hit = 1;
  };

  void setCrit(size_t iTeam) {
    data().flags.getTeam(iTeam).crit = 1;
  };

  void setSecondary(size_t iTeam) {
    data().flags.getTeam(iTeam).secondary = 1;
  };

  void setBlocked(size_t iTeam) {
    data().flags.getTeam(iTeam).blocked = 1;
  };

  void setSwitched(size_t iTeam) {
    data().flags.getTeam(iTeam).switched = 1;
  };

  void setFreeMove(size_t iTeam) {
    data().flags.getTeam(iTeam).free = 1;
  };

  void setWaited(size_t iTeam) {
    data().flags.getTeam(iTeam).wait = 1;
  }

  void setMovedFirst(size_t iTeam) {
    data().flags.getTeam(iTeam).movesFirst = 1;
  }

  void setPruned() { data().setPruned(); };
  void setMerged() { data().setMerged(); };
};


class PKAISHARED PossibleEnvironments : public std::deque<EnvironmentPossibleData> {
public:
  using base_t = std::deque<EnvironmentPossibleData>;
  
  /* Print details of all possible states */
  void printStates() const;
  void printStates(std::ostream& os, const std::string& linePrefix="") const;
  
  /* Selects a state as per the user's choice to evaluate upon */
  ConstEnvironmentPossible stateSelect_index() const {
    size_t indexState;
    return stateSelect_index(indexState);
  }
  ConstEnvironmentPossible stateSelect_index(size_t& indexState) const;
  
  /* selects a state at random, giving greater odds to state with higher probabilities of occurence */
  ConstEnvironmentPossible stateSelect_roulette() const {
    size_t indexState;
    return stateSelect_roulette(indexState);
  }
  ConstEnvironmentPossible stateSelect_roulette(size_t& indexState) const;

  std::vector<ConstEnvironmentPossible> getValidEnvironments(bool sort=false) const;

  EnvironmentPossible at(size_t index) {
    return EnvironmentPossible{*nv_, base_t::at(index)};
  };
  ConstEnvironmentPossible at(size_t index) const {
    return ConstEnvironmentPossible{*nv_, base_t::at(index)};
  };

  /**
   * Returns all states where the environment's bitfield matches the expected
   * values for the given mask.
   */
  std::vector<ConstEnvironmentPossible> where(
      EnvironmentBitfield mask, EnvironmentBitfield expected) const;
  std::vector<EnvironmentPossible> where(
      EnvironmentBitfield mask, EnvironmentBitfield expected);

  /**
   * Returns all states where the provided predicate returns true.
   */
  std::vector<ConstEnvironmentPossible> where(
      const std::function<bool(const ConstEnvironmentPossible&)>& predicate)
      const;
  std::vector<EnvironmentPossible> where(
      const std::function<bool(const ConstEnvironmentPossible&)>& predicate);


  /**
   * Returns all states where the environment's bitfield is a superset of the
   * provided EnvironmentBitfield.
   */
  std::vector<ConstEnvironmentPossible> where(
      EnvironmentBitfield target) const {
    return where(target, target);
  }
  std::vector<EnvironmentPossible> where(EnvironmentBitfield target) {
    return where(target, target);
  }


  std::vector<ConstEnvironmentPossible> whereHit(size_t iTeam) const;
  std::vector<ConstEnvironmentPossible> whereCrit(size_t iTeam) const;
  std::vector<ConstEnvironmentPossible> whereStatus(size_t iTeam) const;
  std::vector<ConstEnvironmentPossible> whereMiss(size_t iTeam) const;
  std::vector<ConstEnvironmentPossible> whereSwitch(size_t iTeam) const;
  std::vector<ConstEnvironmentPossible> whereHitNoCrit(size_t iTeam) const;
  std::vector<ConstEnvironmentPossible> whereHitNoStatus(size_t iTeam) const;

  std::vector<EnvironmentPossible> whereHit(size_t iTeam);
  std::vector<EnvironmentPossible> whereCrit(size_t iTeam);
  std::vector<EnvironmentPossible> whereStatus(size_t iTeam);
  std::vector<EnvironmentPossible> whereMiss(size_t iTeam);
  std::vector<EnvironmentPossible> whereSwitch(size_t iTeam);
  std::vector<EnvironmentPossible> whereHitNoCrit(size_t iTeam);
  std::vector<EnvironmentPossible> whereHitNoStatus(size_t iTeam);

  /**
   * Returns the single most probable state matching the criteria.
   * Throws std::runtime_error if no state matches.
   */
  ConstEnvironmentPossible where1(
      EnvironmentBitfield mask, EnvironmentBitfield expected) const;
  EnvironmentPossible where1(
      EnvironmentBitfield mask, EnvironmentBitfield expected);


  /**
   * Returns the single most probable state matching the predicate.
   * Throws std::runtime_error if no state matches.
   */
  ConstEnvironmentPossible where1(
      const std::function<bool(const ConstEnvironmentPossible&)>& predicate)
      const;
  EnvironmentPossible where1(
      const std::function<bool(const ConstEnvironmentPossible&)>& predicate);

  /**
   * Returns the single most probable state matching the criteria.
   * If no criteria are provided, returns the single most probable state.
   * Throws std::runtime_error if no state matches.
   */
  ConstEnvironmentPossible where1(
      EnvironmentBitfield target = EnvironmentBitfield()) const {
    return where1(target, target);
  }
  EnvironmentPossible where1(
      EnvironmentBitfield target = EnvironmentBitfield()) {
    return where1(target, target);
  }

  ConstEnvironmentPossible where1Hit(size_t iTeam) const;
  ConstEnvironmentPossible where1Crit(size_t iTeam) const;
  ConstEnvironmentPossible where1Status(size_t iTeam) const;
  ConstEnvironmentPossible where1Miss(size_t iTeam) const;
  ConstEnvironmentPossible where1Switch(size_t iTeam) const;
  ConstEnvironmentPossible where1HitNoCrit(size_t iTeam) const;
  ConstEnvironmentPossible where1HitNoStatus(size_t iTeam) const;

  EnvironmentPossible where1Hit(size_t iTeam);
  EnvironmentPossible where1Crit(size_t iTeam);
  EnvironmentPossible where1Status(size_t iTeam);
  EnvironmentPossible where1Miss(size_t iTeam);
  EnvironmentPossible where1Switch(size_t iTeam);
  EnvironmentPossible where1HitNoCrit(size_t iTeam);
  EnvironmentPossible where1HitNoStatus(size_t iTeam);

  size_t getNumUnique() const { return size() - numMerged_; };
  void decrementUnique() { numMerged_++; }

  void setNonvolatileEnvironment(const EnvironmentNonvolatile& nv) {
    nv_ = std::make_shared<const EnvironmentNonvolatile>(nv);
  }
  void setNonvolatileEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv) {
    nv_ = nv;
  }
  
  void clear() {
    base_t::clear();
    numMerged_ = 0;
  }
  
protected:
  std::shared_ptr<const EnvironmentNonvolatile> nv_;
  size_t numMerged_ = 0;
};


PKAISHARED std::ostream& operator <<(std::ostream& os, const ConstEnvironmentPossible& environment);

#endif	/* POSSIBLE_ENVIRONMENT_H */
