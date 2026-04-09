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
#include "environment_bitfield.h"
#include "environment_nonvolatile.h"
#include "environment_volatile.h"
#include "fixedpoint/fixed_class.h"
#include "nonvolatile_volatile.h"
#include "pkai.h"


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

  const EnvironmentBitfield& getBitmask() const { return flags; };
  EnvironmentBitfield& getBitmask() { return flags; };

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

  const EnvironmentBitfield& getBitmask() const { return data().getBitmask(); };

  ConstActorProxy flagsFor(const Actor& actor) const {
    return data().flags.flagsFor(actor);
  }
  ConstActorProxy flagsFor(size_t iTeam, size_t iTeammate) const {
    return data().flags.flagsFor(iTeam, iTeammate);
  }
  ConstActorProxy flagsFor(TEAM team) const {
    return data().flags.flagsFor(team);
  }

  bool isMerged() const { return data().isMerged(); }
  bool isPruned() const { return data().isPruned(); }

  auto yieldActiveActors(size_t movesFirst = TEAM_A) const {
    return getEnv().yieldActiveActors(movesFirst);
  }

  auto yieldActivePokemon(size_t movesFirst = TEAM_A) const {
    return getEnv().yieldActivePokemon(movesFirst);
  }

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

  EnvironmentBitfield& getBitmask() { return data().getBitmask(); };

  using impl_t::flagsFor;
 
  ActorProxy flagsFor(const Actor& actor) { return data().flags.flagsFor(actor); }
  ActorProxy flagsFor(size_t iTeam, size_t iTeammate) {
    return data().flags.flagsFor(iTeam, iTeammate);
  }
  ActorProxy flagsFor(TEAM team) {
    return data().flags.flagsFor(team);
  }

  FixType& getProbability() { return data().getProbability(); };
  const FixType& getProbability() const { return data().getProbability(); };

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

  /* selects the single most likely state deterministically */
  ConstEnvironmentPossible stateSelect_mostLikely() const {
    size_t indexState;
    return stateSelect_mostLikely(indexState);
  }
  ConstEnvironmentPossible stateSelect_mostLikely(size_t& indexState) const;

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

  /**
   * Returns the index of the single most probable state matching the predicate.
   * Throws std::runtime_error if no state matches.
   */
  size_t mostProbableIndex(
      const std::function<bool(const ConstEnvironmentPossible&)>& predicate =
          nullptr) const;

 protected:
  std::shared_ptr<const EnvironmentNonvolatile> nv_;
  size_t numMerged_ = 0;
};


PKAISHARED std::ostream& operator <<(std::ostream& os, const ConstEnvironmentPossible& environment);

#endif	/* POSSIBLE_ENVIRONMENT_H */
