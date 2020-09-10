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
#include <ostream>
#include <memory>
#include <stdint.h>

#include "nonvolatile_volatile.h"
#include "environment_volatile.h"
#include "environment_nonvolatile.h"
#include "../src/fixedpoint/fixed_class.h"


#define ENV_TEAM_HIT 0
#define ENV_TEAM_CRIT 1
#define ENV_TEAM_SECONDARY 2
#define ENV_TEAM_BLOCKED 3
#define ENV_TEAM_SWITCHED 4
#define ENV_TEAM_FREE 5
#define ENV_TEAM_WAIT 6
#define ENV_TEAM_MOVESFIRST 7
#define ENV_PRUNED 15
#define ENV_MERGED 31
#define ENV_TEAM_LAST 16

typedef fixedpoint::fixed_point<30> fixType;

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
  fixType probability;

  /*
   * what type of action occured to create this environment from
   * the previous environment?
   * 
   * if bit n is set to 1:
   * 0 - team a primary effect = hit
   * 1 - team a critical hit = yes
   * 2 - team a secondary effect = hit
   * 3 - a status effect prevented team a from acting
   * 4 - team a pokemon recently switched out
   * 5 - team a dead pokemon recently got a free switch
   * 
   * 8 - team b primary effect = hit
   * 9 - team b critical hit = yes
   * 10 - team b secondary effect = hit
   * 11 - a status effect prevented the team b from acting
   * 12 - team b pokemon recently switched out
   * 13 - team b dead pokemon recently got a free switch out

   * 7 - this environmentPossible has been pruned due to duplicate status
   * 15 - this environmentPossible has been merged with a duplicate environment
   * 
   */
  uint32_t envBitset;


  static EnvironmentPossibleData create(const EnvironmentVolatileData& source, bool doHash = false);

  /* Is the probability of this entity occuring less than the probability of
   the other entity occuring?*/
  bool operator<(const EnvironmentPossibleData& other) const;

  void generateHash();

  const uint64_t& getHash() const { return hash; };

  const fixType& getProbability() const { return probability; };
  fixType& getProbability() { return probability; };

  const uint32_t& getBitmask() const { return envBitset; };
  uint32_t& getBitmask() { return envBitset; };

  void setBit(size_t iBit) { envBitset |= (0x1 << iBit); };
  bool getBit(size_t iBit) const { return ((0x1 << iBit) & envBitset) > 0; }

  void setMerged() {
    setBit(ENV_MERGED);
  };

  void setPruned() {
    setBit(ENV_PRUNED);
  };
  

  bool isPruned() const {
    return getBit(ENV_PRUNED);
  };

  bool isMerged() const {
    return getBit(ENV_MERGED);
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
  void printState(std::ostream& os) const;
  void printEnvironment(std::ostream& os) const;

  operator environmentvolatile_t() const { return environmentvolatile_t{nv(), data().env}; };
  environmentvolatile_t getEnv() const { return environmentvolatile_t{nv(), data().env}; };
  
  const fixType& getProbability() const { return data().getProbability(); };

  const uint64_t& getHash() const { return data().getHash(); };

  const uint32_t& getBitmask() const { return data().getBitmask(); };

  /* has iTeam hit this round? */
  bool hasHit(size_t iTeam) const {
    return data().getBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_HIT);
  };

  /* has iTeam critical hit this round? */
  bool hasCrit(size_t iTeam) const {
    return data().getBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_CRIT);
  };

  /* has iTeam used a secondary effect this round? */
  bool hasSecondary(size_t iTeam) const {
    return data().getBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_SECONDARY);
  };

  /* was iteam's action blocked this round? */
  bool wasBlocked(size_t iTeam) const {
    return data().getBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_BLOCKED);
  };

  /* has iTeam switched this round? */
  bool hasSwitched(size_t iTeam) const {
    return data().getBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_SWITCHED);
  };

  /* has iTeam used a free move this round? */
  bool hasFreeMove(size_t iTeam) const {
    return data().getBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_FREE);
  };

  /* has iTeam used waited this round? */
  bool hasWaited(size_t iTeam) const {
    return data().getBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_WAIT);
  };

  /* has iTeam moved first this round? */
  bool hasMovedFirst(size_t iTeam) const {
    return data().getBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_MOVESFIRST);
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

  fixType& getProbability() { return data().getProbability(); };
  
  void setHit(size_t iTeam) {
    data().setBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_HIT);
  };

  void setCrit(size_t iTeam) {
    data().setBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_CRIT);
  };

  void setSecondary(size_t iTeam) {
    data().setBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_SECONDARY);
  };

  void setBlocked(size_t iTeam) {
    data().setBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_BLOCKED);
  };

  void setSwitched(size_t iTeam) {
    data().setBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_SWITCHED);
  };

  void setFreeMove(size_t iTeam) {
    data().setBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_FREE);
  };

  void setWaited(size_t iTeam) {
    data().setBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_WAIT);
  }

  void setMovedFirst(size_t iTeam) {
    data().setBit(iTeam * ENV_TEAM_LAST + ENV_TEAM_MOVESFIRST);
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

  std::vector<ConstEnvironmentPossible> getValidEnvironments() const;

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
