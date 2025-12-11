/**
 * @file pkCU.h
 * @brief Defines the core battle engine (PkCU) and its internal state machine
 * (PkCUEngine).
 *
 * The PkCU class is the primary interface for simulating Pokemon battles. It
 * takes two actions and a starting environment, and calculates all possible
 * resulting environments, each with an associated probability.
 *
 * The PkCUEngine class implements the internal logic of the battle simulation.
 * It uses a state machine to process a battle turn in stages, following the
 * mechanics of the Pokemon games. This includes determining move priority,
 * calculating damage, handling status effects, and processing secondary move
 * effects. The engine is designed to handle the stochastic nature of battles by
 * creating a stack of possible environments for events like critical hits,
 * misses, and random damage rolls.
 */
#ifndef PKAI_CU_H
#define	PKAI_CU_H

#include "pkai.h"

#include <array>
#include <assert.h>
#include <bitset>
#include <deque>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>
#include <utility>
#include <boost/program_options.hpp>

#include "engine.h"

class PkCUEngine;

/**
 * @name Battle Stages
 * @brief Defines the different stages of the battle engine's state machine.
 *
 * The battle simulation is processed as a sequence of stages. These macros
 * define the identifiers for each stage in the process, from the initial seed
 * to the final hash of the resulting environments.
 * @{
 */
// seed and priority evaluation:
#define STAGE_DNE 0                 /**< Stage does not exist. */
#define STAGE_SEEDED 1              /**< Initial environment has been seeded. */
#define STAGE_PRETURN 2             /**< Before a Pokemon takes its turn. */
// switch evaluation:
#define STAGE_PRESWITCH 3           /**< Before a Pokemon switches out. */
#define STAGE_POSTSWITCH 4          /**< After a Pokemon switches in. */
// pre move evaluation:
#define STAGE_STATUS 5              /**< Before a move is executed, for status effects like paralysis. */
// move damage evaluation:
#define STAGE_MOVEBASE 6            /**< Base move evaluation. */
#define STAGE_SETBASEPOWER 7        /**< Set the base power of the move. */
#define STAGE_SETMOVETYPE 8         /**< Set the type of the move. */
#define STAGE_MODIFYBASEPOWER 9     /**< Modify the base power of the move. */
#define STAGE_MODIFYATTACKPOWER 10  /**< Modify the attack power of the user. */
#define STAGE_MODIFYCRITICALPOWER 11 /**< Modify the power of a critical hit. */
#define STAGE_MODIFYRAWDAMAGE 12    /**< Modify the raw damage calculated. */
#define STAGE_MODIFYSTAB 13         /**< Modify the Same-Type Attack Bonus (STAB). */
#define STAGE_MODIFYTYPERESISTANCE 14 /**< Modify the type resistance of the target. */
#define STAGE_MODIFYITEMPOWER 15    /**< Modify the power based on items. */
#define STAGE_MODIFYHITCHANCE 16    /**< Modify the chance of the move hitting. */
#define STAGE_EVALUATEHITCHANCE 17  /**< Evaluate if the move hits. */
#define STAGE_MODIFYCRITCHANCE 18   /**< Modify the chance of a critical hit. */
#define STAGE_PREDAMAGE 19          /**< Before damage is applied. */
#define STAGE_POSTDAMAGE 20         /**< After damage is applied. */
// post move evaluation:
#define STAGE_POSTMOVE 21           /**< After a move has been executed. */
#define STAGE_PRESECONDARY 22       /**< Before secondary effects are calculated. */
#define STAGE_MODIFYSECONDARYHITCHANCE 23 /**< Modify the hit chance of secondary effects. */
#define STAGE_SECONDARY 24          /**< Secondary effects are being applied. */
#define STAGE_POSTSECONDARY 25      /**< After secondary effects have been applied. */
// post turn status
#define STAGE_POSTTURN 26           /**< After a Pokemon has completed its turn. */
// post round status
#define STAGE_POSTROUND 27          /**< After both Pokemon have completed their turns. */
#define STAGE_FINAL 28              /**< The final stage of the round. */
#define STAGE_HASH 29               /**< The resulting environment is being hashed. */
/** @} */

/**
 * @name Action Validity Checks
 * @brief Defines bitmask flags for checking if an action is valid.
 *
 * These macros are used as indices in a bitset to track the validity of a move
 * or switch action. Plugins can modify the bitset to allow or disallow actions
 * based on game mechanics.
 * @{
 */
#define VALID_MOVE_SELF_ALIVE 0        /**< The user is alive. */
#define VALID_MOVE_TARGET_ALIVE 1      /**< The target is alive. */
#define VALID_MOVE_HAS_PP 2            /**< The move has remaining PP. */
#define VALID_MOVE_FRIENDLY_ALIVE 3    /**< A friendly target is alive. */
#define VALID_MOVE_FRIENDLY_IS_OTHER 4 /**< A friendly target is not the user. */
#define VALID_MOVE_SCRIPT 5            /**< The move is not locked by a script. */
#define VALID_MOVE_SIZE 6              /**< The total number of move validity flags. */

#define VALID_SWAP_FRIENDLY_ALIVE 0    /**< The Pokemon to switch to is alive. */
#define VALID_SWAP_FRIENDLY_IS_OTHER 1 /**< The Pokemon to switch to is not the active Pokemon. */
#define VALID_SWAP_MUST_WAIT 2         /**< A switch is allowed (not during an opponent's free move). */
#define VALID_SWAP_SCRIPT 3            /**< The switch is not locked by a script. */
#define VALID_SWAP_SIZE 4              /**< The total number of swap validity flags. */
/** @} */

/**
 * @struct IsValidResult
 * @brief Represents the result of an action validity check.
 *
 * This struct holds the reason why an action is considered invalid. If the
 * action is valid, the `reason` will be `VALID`. The struct can be evaluated
 * as a boolean to easily check for validity.
 */
struct IsValidResult {
  /**
   * @enum InvalidActionReason
   * @brief Enumerates the possible reasons for an action being invalid.
   */
  enum InvalidActionReason {
    VALID,
    MOVE_INVALID,
    MOVE_TARGET_DEAD,
    MOVE_SELF_DEAD,
    MOVE_NO_PP,
    MOVE_FRIENDLY_TARGET_DEAD,
    MOVE_FRIENDLY_TARGET_SELF,
    MOVE_LOCKED_BY_SCRIPT,
    SWITCH_INVALID_POKEMON,
    SWITCH_TO_SELF,
    SWITCH_POKEMON_DEAD,
    SWITCH_MUST_WAIT,
    SWITCH_LOCKED_BY_SCRIPT,
    WAIT_NOT_ALLOWED,
    STRUGGLE_NOT_ALLOWED,
    ACTION_TYPE_DISABLED,         /**< The action type is disabled (e.g., using an item). */
    INVALID_FRIENDLY_TARGET,      /**< The target for a friendly move is invalid. */
  };

  InvalidActionReason reason;  /**< The reason for the invalid action. */

  /**
   * @brief Constructs an IsValidResult with a specific reason.
   * @param reason The reason for the invalid action.
   */
  IsValidResult(InvalidActionReason reason) : reason(reason) {}

  /**
   * @brief Allows the struct to be evaluated as a boolean.
   * @return `true` if the action is valid, `false` otherwise.
   */
  explicit operator bool() const { return reason == VALID; }
};

/**
 * @brief Converts an IsValidResult to a human-readable string.
 * @param result The IsValidResult to convert.
 * @return A C-string describing the reason for the invalid action.
 */
static const char* invalidActionReasonToString(IsValidResult result) {
  switch (result.reason) {
    case IsValidResult::VALID: return "Valid action";
    case IsValidResult::MOVE_INVALID: return "Move index out of bounds";
    case IsValidResult::MOVE_TARGET_DEAD: return "Target is dead";
    case IsValidResult::MOVE_SELF_DEAD: return "Current pokemon is dead";
    case IsValidResult::MOVE_NO_PP: return "Move has no PP left";
    case IsValidResult::MOVE_FRIENDLY_TARGET_DEAD: return "Friendly target is dead";
    case IsValidResult::MOVE_FRIENDLY_TARGET_SELF: return "Cannot target self with this move";
    case IsValidResult::MOVE_LOCKED_BY_SCRIPT: return "Move locked by script";
    case IsValidResult::SWITCH_INVALID_POKEMON: return "Teammate index is out of bounds";
    case IsValidResult::SWITCH_TO_SELF: return "Cannot switch to self";
    case IsValidResult::SWITCH_POKEMON_DEAD: return "Cannot switch to a dead pokemon";
    case IsValidResult::SWITCH_MUST_WAIT: return "Must wait for opponent's free move";
    case IsValidResult::SWITCH_LOCKED_BY_SCRIPT: return "Switch locked by script";
    case IsValidResult::WAIT_NOT_ALLOWED: return "Wait is not allowed unless opponent has a free move";
    case IsValidResult::STRUGGLE_NOT_ALLOWED: return "Struggle is not a valid action";
    case IsValidResult::ACTION_TYPE_DISABLED: return "Action type disabled";
    case IsValidResult::INVALID_FRIENDLY_TARGET: return "Invalid friendly target";
    default: return "Unknown invalid action reason";
  }
}

/**
 * @struct DamageComponents_t
 * @brief Holds the components used in damage calculation for a single move.
 *
 * This struct is used by the `PkCUEngine` to store intermediate values during
 * the damage calculation process. This allows plugins to modify the components
 * at various stages.
 */
struct DamageComponents_t {
  uint32_t damage;      /**< The calculated damage of the move. */
  uint32_t damageCrit;  /**< The calculated damage if the move is a critical hit. */
  const Type* mType;    /**< The type of the move. */
  fpType cProbability;  /**< The probability of the current environment occurring. */
  fpType tProbability;  /**< The probability of a temporary event, like a move hitting or a secondary effect occurring. */
};

/**
 * @struct MoveBracket
 * @brief Holds the components used to determine move priority.
 */
struct MoveBracket {
  int actionBracket;    /**< The priority bracket of the action. Higher values go first. */
  unsigned int speed;   /**< The speed of the Pokemon, used as a tie-breaker. */
};

using PluginSet = std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>;
using PluginSets = std::array< std::array<PluginSet, 6>, 12>;
using ValidMoveSet = std::bitset<VALID_MOVE_SIZE>;
using ValidSwapSet = std::bitset<VALID_SWAP_SIZE>;

/**
 * @class PkCU
 * @brief The main interface for the Pokemon battle engine.
 *
 * PkCU (Pokemon Calculation Unit) is responsible for simulating a single turn
 * of a Pokemon battle. It takes the current state of the battle and the actions
 * of both players, and computes a set of all possible outcomes with their
 * associated probabilities.
 */
class PKAISHARED PkCU {
public:
  /**
   * @struct Config
   * @brief Configuration options for the PkCU engine.
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
     * @brief If `true`, the engine will not throw an exception for invalid moves.
     *
     * This is useful for scenarios where you want to handle invalid moves
     * gracefully, rather than catching exceptions.
     */
    bool allowInvalidMoves = false;

    Config(){};

    /**
     * @brief Returns a description of the configuration options for Boost.Program_options.
     * @param category The category to display for the options.
     * @param prefix A prefix to add to the option names.
     * @return A `boost::program_options::options_description` object.
     */
    boost::program_options::options_description options(
        const std::string& category="engine configuration",
        std::string prefix = "");
  };

  PkCU(const Config& cfg = Config()): cfg_(cfg) {}
  PkCU(const PkCU& other) = default;
  ~PkCU() {};

  /**
   * @brief Sets the non-volatile environment for the battle.
   *
   * The non-volatile environment contains the static data for the battle,
   * such as the teams and their Pokemon. This method must be called before
   * any simulations are run. It also initializes the plugin sets for the
   * given teams.
   *
   * @param cEnv A shared pointer to the non-volatile environment.
   * @return A reference to this PkCU instance.
   */
  PkCU& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& cEnv);
  PkCU& setEnvironment(const EnvironmentNonvolatile& cEnv) {
    return setEnvironment(std::make_shared<const EnvironmentNonvolatile>(cEnv));
  };

  /**
   * @brief Sets the accuracy of the engine.
   *
   * This is a convenience method for setting the `numRandomEnvironments`
   * configuration option.
   *
   * @param engineAccuracy The number of random environments to create.
   * @return A reference to this PkCU instance.
   */
  PkCU& setAccuracy(size_t engineAccuracy);

  /**
   * @brief Sets whether to allow invalid moves.
   *
   * This is a convenience method for setting the `allowInvalidMoves`
   * configuration option.
   *
   * @param allow `true` to allow invalid moves, `false` to throw exceptions.
   * @return A reference to this PkCU instance.
   */
  PkCU& setAllowInvalidMoves(bool allow = true) { cfg_.allowInvalidMoves = allow; return *this; }
  
  /**
   * @brief Simulates a single turn of a Pokemon battle.
   *
   * Given the current volatile environment and the actions of both players,
   * this method calculates all possible resulting environments and their
   * probabilities. The results are returned as a `PossibleEnvironments`
   * object, which is a collection of `EnvironmentPossible` objects.
   *
   * @param cEnv The current volatile environment.
   * @param actionA The action of player A.
   * @param actionB The action of player B.
   * @return A `PossibleEnvironments` object containing all possible outcomes.
   */
  PossibleEnvironments updateState(
      const ConstEnvironmentVolatile& cEnv, const Action& actionA, const Action& actionB) const;
  PossibleEnvironments updateState(
      const ConstEnvironmentPossible& cEnvP, const Action& actionA, const Action& actionB) const {
    return updateState(cEnvP.getEnv(), actionA, actionB);
  };

  /**
   * @brief Returns the initial volatile state for the configured environment.
   * @return A `ConstEnvironmentVolatile` object representing the initial state.
   */
  ConstEnvironmentVolatile initialState() const {
    return ConstEnvironmentVolatile{*nv_, initialState_};
  };

  /**
   * @brief Returns a list of all valid actions for a given team.
   * @param envV The current volatile environment.
   * @param iTeam The index of the team.
   * @return An `ActionVector` containing all valid actions.
   */
  ActionVector getValidActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const {
    return getValidActionsInRange(envV, iTeam, 0, Action::MOVE_LAST);
  }
  ActionVector getValidMoveActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const {
    return getValidActionsInRange(envV, iTeam, Action::MOVE_0, Action::MOVE_WAIT + 1);
  }
  ActionVector getValidSwapActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const {
    return getValidActionsInRange(envV, iTeam, Action::MOVE_SWITCH, Action::MOVE_SWITCH + 1);
  }
  ActionPairVector getAllValidActions(const ConstEnvironmentVolatile& envV, size_t agentTeam=TEAM_A) const;

  /**
   * @brief Checks if a given action is valid for a team in the current state.
   * @param envV The current volatile environment.
   * @param action The action to check.
   * @param iTeam The index of the team performing the action.
   * @return An `IsValidResult` object indicating if the action is valid.
   */
  IsValidResult isValidAction(const ConstEnvironmentVolatile& envV, const Action& action, size_t iTeam) const;
  IsValidResult isValidAction(const ConstEnvironmentPossible& envV, const Action& action, size_t iTeam) const {
    return isValidAction(envV.getEnv(), action, iTeam);
  }

  /**
   * @brief Checks if the game is over.
   * @param envV The current environment.
   * @return `true` if the game is over, `false` otherwise.
   */
  bool isGameOver(const ConstEnvironmentPossible& envV) const { return isGameOver(envV.getEnv()); }
  bool isGameOver(const ConstEnvironmentVolatile& envV) const {
    return getGameState(envV) != MATCH_MIDGAME;
  }

  /**
   * @brief Returns the current state of the match.
   * @param envV The current environment.
   * @return A `MatchState` enum value indicating the game's status.
   */
  MatchState getGameState(const ConstEnvironmentVolatile& envV) const;
  MatchState getGameState(const ConstEnvironmentPossible& envV) const {
    return getGameState(envV.getEnv());
  }

  static bool isMoveAction(const Action& action) { return action.isMove(); };

  static bool isSwitchAction(const Action& action) { return action.isSwitch(); };

protected:
  Config cfg_;

  /**
   * @brief The non-volatile environment for the current battle.
   *
   * The PkCU instance loads plugins based on the teams in this environment.
   */
  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  /**
   * @brief A multi-dimensional array containing the plugins for all possible matchups.
   *
   * The dimensions represent the active Pokemon for each team.
   */
  PluginSets pluginSets_;

  /**
   * @brief The initial volatile state for the configured non-volatile environment.
   */
  EnvironmentVolatileData initialState_;

  /**
   * @brief Throws an exception if the provided environment's non-volatile state
   * does not match the engine's configured state.
   * @param cEnv The environment to check.
   */
  void guardNonvolatileState(const ConstEnvironmentVolatile& cEnv) const;

  
  ActionVector getValidActionsInRange(
      const ConstEnvironmentVolatile& envV, size_t iTeam, size_t iStart, size_t iEnd) const;

  /**
   * @brief Inserts a plugin into the plugin sets for the appropriate matchups.
   * @param cPlugin The plugin to insert.
   * @param pluginType The type of the plugin.
   * @param iNTeammate The index of the teammate the plugin is associated with.
   * @return The number of plugin sets the plugin was added to.
   */
  size_t insertPluginHandler(plugin_t& cPlugin, size_t pluginType, size_t iNTeammate = SIZE_MAX);

  /**
   * @brief Initializes the plugin sets for the configured non-volatile environment.
   * @return `true` if initialization was successful, `false` otherwise.
   */
  bool initialize();

  const PluginSet& getCPluginSet(const ConstEnvironmentVolatile& cEnv, size_t iTeam) const;

  friend class PkCUEngine;
};

/**
 * @class PkCUEngine
 * @brief The internal state machine for the Pokemon battle engine.
 *
 * This class implements the detailed logic for simulating a single turn of a
 * battle. It is not intended to be used directly, but is instead controlled by
 * the `PkCU` class. The engine processes a turn by advancing through a series
* of stages, each of which corresponds to a specific part of the turn's
 * mechanics.
 */
class PkCUEngine {
protected:
  const PkCU::Config& cfg_;

  /**
   * @brief A reference to the stack of possible environments being generated.
   */
  PossibleEnvironments& stack_;

  const PluginSets& pluginSets_;

  // TODO(@drendleman) - cPluginSet may differ per StackFrame! This cannot be constant!
  /**
   * @brief The current set of plugins for the active matchup.
   */
  const PluginSet* cPluginSet_;

  /**
   * @brief A deque that tracks the current battle stage for each environment on the stack.
   */
  std::deque<uint32_t> stackStage_;

  /**
   * @brief A deque that stores the damage components for each environment on the stack.
   */
  std::deque<DamageComponents_t> damageComponents_;

  /**
   * @brief An array that stores the team indices. `iTeams_[0]` is the current team.
   */
  std::array<size_t, 2> iTeams_;

  /**
   * @brief An array that stores the actions of each team. `actions_[0]` is the current team's action.
   */
  std::array<Action, 2> actions_;

  /**
   * @brief The index of the base environment on the stack that is currently being processed.
   */
  size_t iBase_;

public:
  PkCUEngine(
      const PkCU& cu,
      PossibleEnvironments& stack,
      const EnvironmentVolatileData& initial,
      const Action& actionA,
      const Action& actionB);

  /**
   * @brief The main entry point for the engine. Starts the simulation of the turn.
   */
  void updateState();

  /**
   * @brief Processes the moves of both Pokemon for a single turn.
   */
  void updateState_move();

  /**
   * @brief Determines the priority bracket of a move.
   * @return The priority bracket value.
   */
  int32_t movePriority_Bracket();

  /**
   * @brief Determines the speed of a Pokemon, which is used as a tie-breaker for priority.
   * @return The Pokemon's speed.
   */
  uint32_t movePriority_Speed();

  /**
   * @brief Evaluates a switch action.
   */
  void evaluateMove_switch();

  /**
   * @brief Evaluates pre-move effects, such as status conditions.
   */
  void evaluateMove_preMove();

  /**
   * @brief Evaluates post-move effects, such as secondary effects.
   */
  void evaluateMove_postMove();

  /**
   * @brief Evaluates the damage of a move.
   */
  void evaluateMove_damage();

  /**
   * @brief Evaluates a move that is implemented as a script.
   */
  void evaluateMove_script();

  /**
   * @brief Evaluates end-of-turn effects.
   */
  void evaluateMove_postTurn();

  /**
   * @brief Evaluates end-of-round effects.
   */
  void evaluateRound_end();

  /**
   * @brief Combines similar environments on the stack to reduce redundancy.
   * @return The number of unique environments remaining.
   */
  size_t combineSimilarEnvironments();

  /**
   * @brief Determines which Pokemon moves first.
   * @return The index of the team that moves first, or 2 for a tie.
   */
  uint32_t movePriority();

  /**
   * @brief Evaluates a single move, from pre-move effects to post-move effects.
   */
  void evaluateMove();

  /**
   * @brief Calculates the final damage of a move, including the random damage roll.
   * @param hasCrit `true` if the move is a critical hit, `false` otherwise.
   */
  void calculateDamage(bool hasCrit);

  /**
   * @brief A template function to duplicate the current state on the stack `numEnvironments` times.
   * @tparam numEnvironments The number of copies to create.
   * @param result An array to store the indices of the new environments.
   * @param iState The index of the state to duplicate.
   */
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

  /**
   * @brief Duplicates the current state on the stack, creating two possible outcomes.
   * @param result An array to store the indices of the two new environments.
   * @param probability The probability of the second outcome.
   * @param iState The index of the state to duplicate.
   */
  void duplicateState(std::array<size_t, 2>& result, fpType probability, size_t iState = SIZE_MAX);

  /**
   * @brief Duplicates the current state on the stack, creating three possible outcomes.
   * @param result An array to store the indices of the three new environments.
   * @param probabilityA The probability of the second outcome.
   * @param probabilityB The probability of the third outcome.
   * @param iState The index of the state to duplicate.
   */
  void triplicateState(std::array<size_t, 3>& result, fpType probabilityA, fpType probabilityB, size_t iState = SIZE_MAX);

  /**
   * @brief Gets the current battle stage for the base environment.
   * @return The current stage.
   */
  uint32_t getStackStage() const { return stackStage_[iBase_]; };

  /**
   * @brief Advances the battle stage for the base environment.
   */
  void advanceStackStage() { ++stackStage_[iBase_]; };

  /**
   * @brief Swaps the current team with the other team.
   */
  void swapTeamIndexes();

  const PluginSet& getCPluginSet();
  void setCPluginSet();

  /**
   * @name Accessor Methods
   * @brief A collection of helper methods to access the current state of the battle.
   * @{
   */
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
  MoveVolatile getTMV() { return getTMV(iBase_); }
  MoveVolatile getMV(size_t iState);
  MoveVolatile getTMV(size_t iState);
  DamageComponents_t& getDamageComponent(size_t iStack) { return damageComponents_[iStack]; };
  DamageComponents_t& getDamageComponent() { return damageComponents_[iBase_]; };
  const DamageComponents_t& getDamageComponent() const { return damageComponents_[iBase_]; };
  EnvironmentPossible getBase() { return getStack().at(iBase_); };
  ConstEnvironmentPossible getBase() const { return getStack().at(iBase_); };
  size_t getICTeam() const { return iTeams_[0]; };
  size_t getIOTeam() const { return iTeams_[1]; };
  const Action& getCAction() const { return actions_[0]; };
  const Action& getOAction() const { return actions_[1]; };
  size_t getIBase() const { return iBase_; };
  PossibleEnvironments& getStack() { return stack_; };
  const PossibleEnvironments& getStack() const { return stack_; };
  /** @} */
}; // endOf Engine

#endif	/* PKAI_CU_H */
