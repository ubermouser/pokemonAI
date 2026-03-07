/**
 * @file neo_pkCU.h
 * @brief Defines the new battle engine (NeoPkCU) and its internal state machine
 * (NeoPkCUEngine) with stub implementations.
 */
#ifndef NEO_PKAI_CU_H
#define NEO_PKAI_CU_H

#include <boost/program_options.hpp>
#include <memory>
#include <vector>

#include "engine.h"
#include "pkCU_types.h"
#include "pkai.h"

// clang-format off
/**
 * @name Battle Stages
 * @brief Defines the different stages of the battle engine's state machine.
 *
 * The battle simulation is processed as a sequence of stages. These identifiers
 * define each stage in the process, from the initial seed to the final hash
 * of the resulting environments.
 * @{
 */
enum StageType : int {
  // seed and priority evaluation:
  DNE = 0,                 /**< Stage does not exist. */
  SEEDED = 1,              /**< Initial environment has been seeded. */
  PRETURN = 2,             /**< Before a Pokemon takes its turn. */
  // switch evaluation:
  PRESWITCH = 3,           /**< Before a Pokemon switches out. */
  POSTSWITCH = 4,          /**< After a Pokemon switches in. */
  // pre move evaluation:
  STATUS = 5,              /**< Before a move is executed, for status effects like paralysis. */
  // move damage evaluation:
  MOVEBASE = 6,            /**< Base move evaluation. */
  MODIFYHITCHANCE = 7,     /**< Modify the chance of the move hitting. */
  EVALUATEHITCHANCE = 8,   /**< Evaluate if the move hits. */
  MODIFYCRITCHANCE = 9,    /**< Modify the chance of a critical hit. */
  EVALUATECRITCHANCE = 10, /**< Evaluate if the move crits. */
  SETBASEPOWER = 11,       /**< Set the base power of the move. */
  SETMOVETYPE = 12,        /**< Set the type of the move. */
  MODIFYBASEPOWER = 13,    /**< Modify the base power of the move. */
  MODIFYATTACKPOWER = 14,  /**< Modify the attack power of the user. */
  MODIFYCRITICALPOWER = 15, /**< Modify the power of a critical hit. */
  MODIFYRAWDAMAGE = 16,    /**< Modify the raw damage calculated. */
  MODIFYSTAB = 17,         /**< Modify the Same-Type Attack Bonus (STAB). */
  MODIFYTYPERESISTANCE = 18, /**< Modify the type resistance of the target. */
  MODIFYITEMPOWER = 19,    /**< Modify the power based on items. */
  PREDAMAGE = 20,          /**< Before damage is applied. */
  POSTDAMAGE = 21,         /**< After damage is applied. */
  // post move evaluation:
  POSTMOVE = 22,           /**< After a move has been executed. */
  PRESECONDARY = 23,       /**< Before secondary effects are calculated. */
  MODIFYSECONDARYHITCHANCE = 24, /**< Modify the hit chance of secondary effects. */
  SECONDARY = 25,          /**< Secondary effects are being applied. */
  POSTSECONDARY = 26,      /**< After secondary effects have been applied. */
  // post turn status
  POSTTURN = 27,           /**< After a Pokemon has completed its turn. */
  // post round status
  POSTROUND = 28,          /**< After both Pokemon have completed their turns. */
  FINAL = 29,              /**< The final stage of the round. */
  HASH = 30                /**< The resulting environment is being hashed. */
};
/** @} */
// clang-format on


class NeoPkCUEngine;

class PKAISHARED NeoPkCU {
public:
 /**
  * @struct Config
  * @brief Configuration options for the NeoPkCU engine.
  */
 struct Config {
   /**
    * @brief The number of random environments to create for stochastic events.
    *
    * This controls the accuracy of the simulation for events with random
    * outcomes, such as damage rolls. A higher number will produce more
    * accurate results but will be more computationally expensive. The value
    * should be between 1 and 16.
    */
   size_t numRandomEnvironments = 1;

   /**
    * @brief If `true`, the engine will not throw an exception for invalid
    * moves.
    *
    * This is useful for scenarios where you want to handle invalid moves
    * gracefully, rather than catching exceptions.
    */
   bool allowInvalidMoves = false;

   Config(){};

   /**
    * @brief Returns a description of the configuration options for
    * Boost.Program_options.
    * @param category The category to display for the options.
    * @param prefix A prefix to add to the option names.
    * @return A `boost::program_options::options_description` object.
    */
   boost::program_options::options_description options(
       const std::string& category = "engine configuration",
       std::string prefix = "");
 };

  NeoPkCU(const Config& cfg = Config());
  NeoPkCU(const NeoPkCU& other);
  ~NeoPkCU();

  NeoPkCU* clone() const;

  NeoPkCU& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& cEnv);
  NeoPkCU& setEnvironment(const EnvironmentNonvolatile& cEnv);
  NeoPkCU& setAccuracy(size_t engineAccuracy);
  NeoPkCU& setAllowInvalidMoves(bool allow = true);
  
  PossibleEnvironments updateState(
      const ConstEnvironmentVolatile& cEnv, const Action& actionA, const Action& actionB) const;
  PossibleEnvironments updateState(
      const ConstEnvironmentPossible& cEnvP, const Action& actionA, const Action& actionB) const;

  ConstEnvironmentVolatile initialState() const;

  ActionVector getValidActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const;
  ActionVector getValidMoveActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const;
  ActionVector getValidSwapActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const;
  ActionPairVector getAllValidActions(const ConstEnvironmentVolatile& envV, size_t agentTeam=TEAM_A) const;

  IsValidResult isValidAction(const ConstEnvironmentVolatile& envV, const Action& action, size_t iTeam) const;
  IsValidResult isValidAction(const ConstEnvironmentPossible& envV, const Action& action, size_t iTeam) const;

  bool isGameOver(const ConstEnvironmentPossible& envV) const;
  bool isGameOver(const ConstEnvironmentVolatile& envV) const;

  MatchState getGameState(const ConstEnvironmentVolatile& envV) const;
  MatchState getGameState(const ConstEnvironmentPossible& envV) const;

protected:
  Config cfg_;
  std::shared_ptr<const EnvironmentNonvolatile> nv_;
  EnvironmentVolatileData initialStateData_;

  friend class NeoPkCUEngine;
};

class NeoPkCUEngine {
public:
  NeoPkCUEngine(
      const NeoPkCU& cu,
      PossibleEnvironments& stack,
      const EnvironmentVolatileData& initial,
      const Action& actionA,
      const Action& actionB);

  void updateState();

  // Add other methods as needed to match Legacy if required for plugin compatibility, 
  // but for now, this is the core interface.
};

#endif /* NEO_PKAI_CU_H */
