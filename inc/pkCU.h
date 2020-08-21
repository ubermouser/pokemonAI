#ifndef PKAI_CU_H
#define	PKAI_CU_H

#include "../inc/pkai.h"

#include <array>
#include <assert.h>
#include <deque>
#include <memory>
#include <stdint.h>
#include <vector>
#include <utility>

#include "environment_nonvolatile.h"
#include "environment_possible.h"
#include "environment_volatile.h"
#include "pluggable.h"

class PkCUEngine;

// seed and priority evaluation:
#define STAGE_DNE 0
#define STAGE_SEEDED 1
#define STAGE_PRETURN 2
// switch evaluation:
#define STAGE_PRESWITCH 3
#define STAGE_POSTSWITCH 4
// pre move evaluation:
#define STAGE_STATUS 5
// move damage evaluation:
#define STAGE_MOVEBASE 6
#define STAGE_SETBASEPOWER 7
#define STAGE_SETMOVETYPE 8
#define STAGE_MODIFYBASEPOWER 9
#define STAGE_MODIFYATTACKPOWER 10
#define STAGE_MODIFYCRITICALPOWER 11
#define STAGE_MODIFYRAWDAMAGE 12
#define STAGE_MODIFYSTAB 13
#define STAGE_MODIFYTYPERESISTANCE 14
#define STAGE_MODIFYITEMPOWER 15
#define STAGE_MODIFYHITCHANCE 16
#define STAGE_EVALUATEHITCHANCE 17
#define STAGE_MODIFYCRITCHANCE 18
#define STAGE_PREDAMAGE 19
#define STAGE_POSTDAMAGE 20
// post move evaluation:
#define STAGE_POSTMOVE 21
#define STAGE_PRESECONDARY 22
#define STAGE_MODIFYSECONDARYHITCHANCE 23
#define STAGE_SECONDARY 24
#define STAGE_POSTSECONDARY 25
/// post turn status
#define STAGE_POSTTURN 26
// post round status
#define STAGE_POSTROUND 27
#define STAGE_FINAL 28
#define STAGE_HASH 29

struct DamageComponents_t {
  uint32_t damage;
  uint32_t damageCrit;
  const Type* mType;
  fpType cProbability;
  fpType tProbability;
};

struct MoveBracket {
  int actionBracket;
  unsigned int speed;
};

using PluginSet = std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>;
using PluginSets = std::array< std::array<PluginSet, 6>, 12>;

class PKAISHARED PkCU {
public:
  struct Config {
    /* number of random environments to create per hit/crit 1-16 */
    size_t numRandomEnvironments = 1;

    /* When true, providing invalid actions will not cause an exception to be thrown. */
    bool allowInvalidMoves = false;

    Config(){};
  };

  PkCU(const Config& cfg = Config()): cfg_(cfg) {}
  PkCU(const PkCU& other) = default;
  ~PkCU() {};

  /* pkCU stores a reference to the environment at cEnvironment. */
  PkCU& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& cEnv);
  PkCU& setEnvironment(const EnvironmentNonvolatile& cEnv) {
    return setEnvironment(std::make_shared<const EnvironmentNonvolatile>(cEnv));
  };

  /* sets accuracy of pkCU */
  PkCU& setAccuracy(size_t engineAccuracy);

  /* When true, providing invalid actions will not cause an exception to be thrown. */
  PkCU& setAllowInvalidMoves(bool allow = true) { cfg_.allowInvalidMoves = allow; return *this; }
  
  /*
   * given actions A and B and currentEnvironment, generate
   * resultEnvironment vector - what would likely happen if this
   * sequence of actions took place
   * 
   * Source: http://www.smogon.com/dp/articles/damage_formula
   * 
   * action:
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   * 
   * returns: number of unique environments in vector
   */
  PossibleEnvironments updateState(
      const ConstEnvironmentVolatile& cEnv, size_t actionA, size_t actionB) const;
  PossibleEnvironments updateState(
      const ConstEnvironmentPossible& cEnvP, size_t actionA, size_t actionB) const {
    return updateState(cEnvP.getEnv(), actionA, actionB);
  };

  /* Seed an initial state from an EnvironmentNonvolatile, then return its volatile state. */
  ConstEnvironmentVolatile initialState() const;

  /* Return a collection of all valid actions for the given state. */
  std::vector<size_t> getValidActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const;
  std::vector<std::array<size_t, 2> > getAllValidActions(const ConstEnvironmentVolatile& envV) const;

  /* determines whether a given action is a selectable one, given the current state */
  bool isValidAction(const ConstEnvironmentVolatile& envV, size_t action, size_t iTeam) const;
  bool isValidAction(const ConstEnvironmentPossible& envV, size_t action, size_t iTeam) const {
    return isValidAction(envV.getEnv(), action, iTeam);
  }

  /* determines whether a game has ended, given the current state. Returns an enum of the game's current status */
  MatchState isGameOver(const ConstEnvironmentVolatile& envV) const;
  MatchState isGameOver(const ConstEnvironmentPossible& envV) const {
    return isGameOver(envV.getEnv());
  }

  static bool isMoveAction(size_t iAction) {
    return (iAction >= AT_MOVE_0 && iAction <= AT_MOVE_3) || (iAction == AT_MOVE_STRUGGLE);
  };

  static bool isSwitchAction(size_t iAction) {
    return (iAction >= AT_SWITCH_0 && iAction <= AT_SWITCH_5);
  };

protected:
  Config cfg_;

  /* the current nonvolatile environment; pkCU loads plugins only for these two teams to fight */
  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  /* array containing all possible matchups and the plugins they may call upon */
  PluginSets pluginSets_;

  /* The default state for a given nonvolatile state */
  EnvironmentVolatileData initialState_;

  /* if iTeam = SIZE_MAX, insert for both teams. if iCTeammate =  SIZE_MAX, insert for all teammates. True if non-duplicate */
  size_t insertPluginHandler(plugin_t& cPlugin, size_t pluginType, size_t iNTeammate = SIZE_MAX);

  /* fully seeds the pluginset vector */
  bool initialize();

  friend class PkCUEngine;
};


class PkCUEngine {
protected:
  const PkCU::Config& cfg_;

  /* stack of environment_possible objects */
  PossibleEnvironments& stack_;

  const PluginSets& pluginSets_;

  // TODO(@drendleman) - cPluginSet may differ per StackFrame! This cannot be constant!
  /* the current matchup, based on which two pokemon are active */
  const PluginSet* cPluginSet_;

  /* the stage of computation each element on the stack is in */
  std::deque<uint32_t> stackStage_;

  /* when executing the default damage pathway, these components are used for persistent data */
  std::deque<DamageComponents_t> damageComponents_;

  /* an array of which team is going when. 0 is current team, 1 is other team */
  std::array<size_t, 2> iTeams_;

  /* an array of each team's action. 0 is current team, 1 is other team */
  std::array<size_t, 2> iActions_;

  /* iterator - the environment that the current operation is creating values from */
  size_t iBase_;

public:
  PkCUEngine(
      const PkCU& cu,
      PossibleEnvironments& stack,
      const EnvironmentVolatileData& initial,
      size_t actionA,
      size_t actionB);

  void updateState();

  void updateState_move();

  /*
   * Determine the priority of a given action by a given team's
   * currently active pokemon
   *
   * action:
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   *
   * team:
   * 0 - team A
   * 1 - team B
   *
   * Source: http://www.smogon.com/dp/articles/move_priority
   */
  int32_t movePriority_Bracket();

  uint32_t movePriority_Speed();

  void evaluateMove_switch();

  void evaluateMove_preMove();

  void evaluateMove_postMove();

  /*
   *
   * evaluates all possible outcomes for a given move, and calculates
   * their probability of occurrence.
   *
   * @param action:
   * AT_MOVE_0-3: pokemon's move
   *
   * @param team:
   * 0 - team A
   * 1 - team B
   *
   * @return TRUE if success, FALSE if failure
   *
   * Source: http://www.smogon.com/dp/articles/damage_formula
   */
  void evaluateMove_damage();

  void evaluateMove_script();

  /* do things that occur after a turn but before the end of every round */
  void evaluateMove_postTurn();

  /* do things that occur at the end of every round */
  void evaluateRound_end();

  /* returns the number of unique environments in the result array, applies the isPruned tag to pruned environments */
  size_t combineSimilarEnvironments();

  /*
   * Determine who goes first, given two pokemon and their move
   *
   * @param action:
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   *
   * @return 0 if teamA wins, 1 if teamB wins, 2 if tie
   */
  uint32_t movePriority();

  /*
   *
   * evaluates damage and to-hit components of a standard move, calculates
   * their probability of occurance
   *
   * @param action:
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   *
   * @param team:
   * 0 - team A
   * 1 - team B
   *
   * @return TRUE if success, FALSE if failure
   *
   * Source: http://www.smogon.com/dp/articles/damage_formula
   * Source: http://www.smogon.com/dp/articles/status
   */
  void evaluateMove();

  /*
   * calculates stochastic component of standard damage dealt
   */
  void calculateDamage(bool hasCrit);

  template<size_t numEnvironments>
  void nPlicateState(std::array<size_t, numEnvironments>& result, size_t iState = SIZE_MAX)
  {
    if (iState == SIZE_MAX) { iState = iBase_; }
    PossibleEnvironments& stack = getStack();

    //assert((stack.size() + numEnvironments) <= MAXSTACKSIZE);

    result[0] = iState;
    uint32_t baseStage = stackStage_[iState];
    const DamageComponents_t& baseComponent = damageComponents_[iState];
    for (size_t iEnvironment = 1; iEnvironment < numEnvironments; ++iEnvironment)
    {
      size_t cSize = stack.size();

      result[iEnvironment] = cSize;
      stackStage_.push_back(baseStage);
      damageComponents_.push_back(baseComponent);

      stack.push_back(stack[iState]);
    }
  };

  void duplicateState(std::array<size_t, 2>& result, fpType probability, size_t iState = SIZE_MAX);

  void triplicateState(std::array<size_t, 3>& result, fpType probabilityA, fpType probabilityB, size_t iState = SIZE_MAX);

  uint32_t getStackStage() const { return stackStage_[iBase_]; };

  void advanceStackStage() { ++stackStage_[iBase_]; };

  void swapTeamIndexes();

  const PluginSet& getCPluginSet();
  void setCPluginSet();

  // TODO(@drendleman) consider memoizing these in a stack (are they expensive to create?)
  TeamVolatile getTV() { return getTV(iBase_); }
  TeamVolatile getTTV() { return getTTV(iBase_); }
  TeamVolatile getTV(size_t iState);
  TeamVolatile getTTV(size_t iState);
  PokemonVolatile getPKV() { return getPKV(iBase_); }
  PokemonVolatile getTPKV() { return getTPKV(iBase_); }
  PokemonVolatile getPKV(size_t iState);
  PokemonVolatile getTPKV(size_t iState);
  MoveVolatile getMV() { return getMV(iBase_); }
  MoveVolatile getMV(size_t iState);
  MoveVolatile getTMV();
  MoveVolatile getTMV(size_t iState);
  DamageComponents_t& getDamageComponent(size_t iStack) { return damageComponents_[iStack]; };
  DamageComponents_t& getDamageComponent() { return damageComponents_[iBase_]; };
  const DamageComponents_t& getDamageComponent() const { return damageComponents_[iBase_]; };
  EnvironmentPossible getBase() { return getStack().at(iBase_); };
  ConstEnvironmentPossible getBase() const { return getStack().at(iBase_); };
  size_t getICTeam() const { return iTeams_[0]; };
  size_t getIOTeam() const { return iTeams_[1]; };
  size_t getICAction() const { return iActions_[0]; };
  size_t getIOAction() const { return iActions_[1]; };
  size_t getIBase() const { return iBase_; };
  PossibleEnvironments& getStack() { return stack_; };
  const PossibleEnvironments& getStack() const { return stack_; };
}; // endOf Engine

#endif	/* PKAI_CU_H */
