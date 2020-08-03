/* 
 * File:   possible_environment.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:25 PM
 */

#ifndef POSSIBLE_ENVIRONMENT_H
#define	POSSIBLE_ENVIRONMENT_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>

#include "../src/fixedpoint/fixed_class.h"
#include "../inc/environment_volatile.h"

typedef fixedpoint::fixed_point<30> fixType;
class EnvironmentNonvolatile;

union PKAISHARED EnvironmentPossible
{
  struct
  {
    /*
     * the environment this possible_environment represents
     */
    EnvironmentVolatile env;

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
  } data;
  uint64_t raw[18];

  static EnvironmentPossible create(const EnvironmentVolatile& source, bool doHash = false);

  /* Is the probability of this entity occuring less than the probability of 
   the other entity occuring?*/
  bool operator<(const EnvironmentPossible& other) const;
  
  /* print details of a single state */
  void printState(
      const EnvironmentNonvolatile& envNV, size_t iState=SIZE_MAX, size_t iPly=SIZE_MAX) const;

  const EnvironmentVolatile& getEnv() const { return data.env; };

  EnvironmentVolatile& getEnv() { return data.env; }

  const fixType& getProbability() const { return data.probability; };

  fixType& getProbability() { return data.probability; };

  const uint32_t& getBitmask() const { return data.envBitset; };

  uint32_t& getBitmask() { return data.envBitset; };

  const uint64_t& getHash() const { return data.hash; };

  void setHit(size_t iTeam)
  {
    data.envBitset = data.envBitset | (0x1<<(0 + iTeam*8));
  };

  void setCrit(size_t iTeam)
  {
    data.envBitset = data.envBitset | (0x1<<(1 + iTeam*8));
  };

  void setSecondary(size_t iTeam)
  {
    data.envBitset = data.envBitset | (0x1<<(2 + iTeam*8));
  };

  void setBlocked(size_t iTeam)
  {
    data.envBitset = data.envBitset | (0x1<<(3 + iTeam*8));
  };

  void setSwitched(size_t iTeam)
  {
    data.envBitset = data.envBitset | (0x1<<(4 + iTeam*8));
  };

  void setFreeMove(size_t iTeam)
  {
    data.envBitset = data.envBitset | (0x1<<(5 + iTeam*8));
  };

  void setWaited(size_t iTeam)
  {
    data.envBitset = data.envBitset | (0x1<<(6 + iTeam*8));
  }

  void setMerged()
  {
    data.envBitset = data.envBitset | (0x1<<(15));
  };

  void setPruned()
  {
    data.envBitset = data.envBitset | (0x1<<(7));
  };

  /* has iTeam hit this round? */
  bool hasHit(size_t iTeam) const
  {
    return (0x1<<(0 + iTeam*8) & data.envBitset) > 0;
  };

  /* has iTeam critical hit this round? */
  bool hasCrit(size_t iTeam) const
  {
    return (0x1<<(1 + iTeam*8) & data.envBitset) > 0;
  };

  /* has iTeam used a secondary effect this round? */
  bool hasSecondary(size_t iTeam) const
  {
    return (0x1<<(2 + iTeam*8) & data.envBitset) > 0;
  };

  /* was iteam's action blocked this round? */
  bool wasBlocked(size_t iTeam) const
  {
    return (0x1<<(3 + iTeam*8) & data.envBitset) > 0;
  };

  /* has iTeam switched this round? */
  bool hasSwitched(size_t iTeam) const
  {
    return (0x1<<(4 + iTeam*8) & data.envBitset) > 0;
  };
  
  /* has iTeam used a free move this round? */
  bool hasFreeMove(size_t iTeam) const
  {
    return (0x1<<(5 + iTeam*8) & data.envBitset) > 0;
  };

  /* has iTeam used waited this round? */
  bool hasWaited(size_t iTeam) const
  {
    return (0x1<<(6 + iTeam*8) & data.envBitset) > 0;
  };

  bool isMerged() const
  {
    return (0x1<<(15) & data.envBitset) > 0;
  };

  bool isPruned() const
  {
    return (0x1<<(7) & data.envBitset) > 0;
  };

  void generateHash();
};


class PKAISHARED PossibleEnvironments : public std::vector<EnvironmentPossible> {
public:
  using base_t = std::vector<EnvironmentPossible>;
  
  /* Print details of all possible states */
  void printStates(const EnvironmentNonvolatile& envNV, size_t iPly=SIZE_MAX) const;
  
  /* Selects a state as per the user's choice to evaluate upon */
  const EnvironmentPossible* stateSelect_index(const EnvironmentNonvolatile& envNV) const {
    printStates(envNV);
    return stateSelect_index();
  }
  const EnvironmentPossible* stateSelect_index() const {
    size_t indexState;
    return stateSelect_index(indexState);
  }
  const EnvironmentPossible* stateSelect_index(size_t& indexState) const;
  
  /* selects a state at random, giving greater odds to state with higher probabilities of occurence */
  const EnvironmentPossible& stateSelect_roulette() const {
    size_t indexState;
    return stateSelect_roulette(indexState);
  }
  const EnvironmentPossible& stateSelect_roulette(size_t& indexState) const;
  
  size_t getNumUnique() const { return size() - numMerged; };
  void decrementUnique() { numMerged++; }
  
  void clear() {
    base_t::clear();
    numMerged = 0;
  }
  
protected:
  size_t numMerged = 0;
};


class PKAISHARED envP_print
{
private:
  const EnvironmentNonvolatile& envNV;
  const EnvironmentPossible& envP;
  size_t agentTeam;

public:
  envP_print(const EnvironmentNonvolatile& _envNV, const EnvironmentPossible& _envP, size_t _agentTeam = TEAM_A)
    : envNV(_envNV),
    envP(_envP),
    agentTeam(_agentTeam)
  {
  };

  friend PKAISHARED std::ostream& operator <<(std::ostream& os, const envP_print& environment);
};


PKAISHARED std::ostream& operator <<(std::ostream& os, const envP_print& environment);

#endif	/* POSSIBLE_ENVIRONMENT_H */

