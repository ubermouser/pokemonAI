/* 
 * File:   possible_environment.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:25 PM
 */

#ifndef POSSIBLE_ENVIRONMENT_H
#define	POSSIBLE_ENVIRONMENT_H

#include "pkai.h"

#include <deque>
#include <iosfwd>
#include <memory>
#include <stdint.h>

#include "nonvolatile_volatile.h"
#include "environment_volatile.h"
#include "environment_nonvolatile.h"
#include "fixedpoint/fixed_class.h"


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

  const TeamEnvironmentFlags& getTeam(size_t i) const {
    return (i == 0) ? bits.team0 : bits.team1;
  }

  TeamEnvironmentFlags& getTeam(size_t i) {
    return (i == 0) ? bits.team0 : bits.team1;
  }
};

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

  void generateHash();

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
};


class PKAISHARED ConstEnvironmentPossible: public EnvironmentPossibleImpl<ConstEnvironmentVolatile, const EnvironmentPossibleData> {
public:
  using impl_t::impl_t;

  // TODO(@drendleman) a workaround for stateSelect having to return a NULL State
  explicit ConstEnvironmentPossible(nonvolatile_t& nv);
  bool isEmpty() const;
};


class PKAISHARED EnvironmentPossible: public EnvironmentPossibleImpl<EnvironmentVolatile, EnvironmentPossibleData> {
public:
  using impl_t::impl_t;

  operator ConstEnvironmentPossible() const { return ConstEnvironmentPossible{nv(), data()}; };

  uint32_t& getBitmask() { return data().getBitmask(); };

  FixType& getProbability() { return data().getProbability(); };

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
