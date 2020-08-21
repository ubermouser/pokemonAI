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

  void setMerged()
  {
    envBitset = envBitset | (0x1<<(15));
  };

  void setPruned()
  {
    envBitset = envBitset | (0x1<<(7));
  };
  

  bool isPruned() const
  {
    return (0x1<<(7) & envBitset) > 0;
  };
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
  void printState(size_t iState=SIZE_MAX, size_t iPly=SIZE_MAX) const;
  void printEnvironment(std::ostream& os) const;

  operator environmentvolatile_t() const { return environmentvolatile_t{nv(), data().env}; };
  environmentvolatile_t getEnv() const { return environmentvolatile_t{nv(), data().env}; };
  
  const fixType& getProbability() const { return data().getProbability(); };

  const uint64_t& getHash() const { return data().getHash(); };

  const uint32_t& getBitmask() const { return data().getBitmask(); };

  /* has iTeam hit this round? */
  bool hasHit(size_t iTeam) const
  {
    return (0x1<<(0 + iTeam*8) & data().envBitset) > 0;
  };

  /* has iTeam critical hit this round? */
  bool hasCrit(size_t iTeam) const
  {
    return (0x1<<(1 + iTeam*8) & data().envBitset) > 0;
  };

  /* has iTeam used a secondary effect this round? */
  bool hasSecondary(size_t iTeam) const
  {
    return (0x1<<(2 + iTeam*8) & data().envBitset) > 0;
  };

  /* was iteam's action blocked this round? */
  bool wasBlocked(size_t iTeam) const
  {
    return (0x1<<(3 + iTeam*8) & data().envBitset) > 0;
  };

  /* has iTeam switched this round? */
  bool hasSwitched(size_t iTeam) const
  {
    return (0x1<<(4 + iTeam*8) & data().envBitset) > 0;
  };

  /* has iTeam used a free move this round? */
  bool hasFreeMove(size_t iTeam) const
  {
    return (0x1<<(5 + iTeam*8) & data().envBitset) > 0;
  };

  /* has iTeam used waited this round? */
  bool hasWaited(size_t iTeam) const
  {
    return (0x1<<(6 + iTeam*8) & data().envBitset) > 0;
  };

  bool isMerged() const
  {
    return (0x1<<(15) & data().envBitset) > 0;
  };

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
  
  void setHit(size_t iTeam)
  {
    data().envBitset = data().envBitset | (0x1<<(0 + iTeam*8));
  };

  void setCrit(size_t iTeam)
  {
    data().envBitset = data().envBitset | (0x1<<(1 + iTeam*8));
  };

  void setSecondary(size_t iTeam)
  {
    data().envBitset = data().envBitset | (0x1<<(2 + iTeam*8));
  };

  void setBlocked(size_t iTeam)
  {
    data().envBitset = data().envBitset | (0x1<<(3 + iTeam*8));
  };

  void setSwitched(size_t iTeam)
  {
    data().envBitset = data().envBitset | (0x1<<(4 + iTeam*8));
  };

  void setFreeMove(size_t iTeam)
  {
    data().envBitset = data().envBitset | (0x1<<(5 + iTeam*8));
  };

  void setWaited(size_t iTeam)
  {
    data().envBitset = data().envBitset | (0x1<<(6 + iTeam*8));
  }

  void setPruned() { data().setPruned(); };
  void setMerged() { data().setMerged(); };
};


class PKAISHARED PossibleEnvironments : public std::deque<EnvironmentPossibleData> {
public:
  using base_t = std::deque<EnvironmentPossibleData>;
  
  /* Print details of all possible states */
  void printStates(size_t iPly=SIZE_MAX) const;
  
  /* Selects a state as per the user's choice to evaluate upon */
  ConstEnvironmentPossible stateSelect_index(bool doPrint=false) const {
    if (doPrint) { printStates(); }
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
