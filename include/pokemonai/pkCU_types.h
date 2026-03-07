#ifndef PKCU_TYPES_H
#define PKCU_TYPES_H

#include "pkai.h"
#include "pluggable.h"
#include <array>
#include <bitset>
#include <vector>

class Type;

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
 */
struct DamageComponents_t {
  uint32_t damage;      /**< The calculated damage of the move. */
  uint32_t category;    /**< The damage category of the move
                           (Physical/Special/Fixed). */
  const Type* mType;    /**< The type of the move. */
  FixType cProbability; /**< The probability of the current environment
                           occurring. */
  FixType tProbability; /**< The probability of a temporary event, like a move
                           hitting or a secondary effect occurring. */
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

#endif // PKCU_TYPES_H
