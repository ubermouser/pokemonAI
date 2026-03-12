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
// clang-format off
#define VALID_MOVE_SELF_ALIVE 0        /**< The user is alive. */
#define VALID_MOVE_TARGET_ALIVE 1      /**< The target is alive. */
#define VALID_MOVE_HAS_PP 2            /**< The move has remaining PP. */
#define VALID_MOVE_FRIENDLY_ALIVE 3    /**< A friendly target is alive. */
#define VALID_MOVE_FRIENDLY_IS_OTHER 4 /**< A friendly target is not the user. */
#define VALID_MOVE_SCRIPT 5            /**< The move is not locked by a script. */
#define VALID_MOVE_ACTOR_ACTIVE 6      /**< The user is currently active on the field. */
#define VALID_MOVE_SIZE 7              /**< The total number of move validity flags. */

#define VALID_SWAP_FRIENDLY_ALIVE 0    /**< The Pokemon to switch to is alive. */
#define VALID_SWAP_FRIENDLY_IS_OTHER 1 /**< The Pokemon to switch to is not the active Pokemon. */
#define VALID_SWAP_MUST_WAIT 2         /**< A switch is allowed (not during an opponent's free move). */
#define VALID_SWAP_SCRIPT 3            /**< The switch is not locked by a script. */
#define VALID_SWAP_TARGET_INACTIVE 4   /**< The switch target is currently not active on the field */
#define VALID_SWAP_SIZE 5              /**< The total number of swap validity flags. */
// clang-format on
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
    MOVE_ACTOR_NOT_ACTIVE,
    MOVE_INVALID,
    MOVE_TARGET_DEAD,
    MOVE_SELF_DEAD,
    MOVE_NO_PP,
    MOVE_FRIENDLY_TARGET_DEAD,
    MOVE_FRIENDLY_TARGET_SELF,
    MOVE_LOCKED_BY_SCRIPT,
    SWITCH_INVALID_POKEMON,
    SWITCH_TO_SELF,
    SWITCH_ACTIVE_POKEMON,
    SWITCH_POKEMON_DEAD,
    SWITCH_MUST_WAIT,
    SWITCH_LOCKED_BY_SCRIPT,
    WAIT_NOT_ALLOWED,
    STRUGGLE_NOT_ALLOWED,
    ACTION_TYPE_DISABLED,    /**< The action type is disabled (e.g., using an
                                item). */
    INVALID_FRIENDLY_TARGET, /**< The target for a friendly move is invalid. */
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
    // clang-format off
    case IsValidResult::VALID: return "Valid action";
    case IsValidResult::MOVE_ACTOR_NOT_ACTIVE: return "Actor is currently not in play";
    case IsValidResult::MOVE_INVALID: return "Move index out of bounds";
    case IsValidResult::MOVE_TARGET_DEAD: return "Target is dead";
    case IsValidResult::MOVE_SELF_DEAD: return "Current pokemon is dead";
    case IsValidResult::MOVE_NO_PP: return "Move has no PP left";
    case IsValidResult::MOVE_FRIENDLY_TARGET_DEAD: return "Friendly target is dead";
    case IsValidResult::MOVE_FRIENDLY_TARGET_SELF: return "Cannot target self with this move";
    case IsValidResult::MOVE_LOCKED_BY_SCRIPT: return "Move locked by script";
    case IsValidResult::SWITCH_INVALID_POKEMON: return "Teammate index is out of bounds";
    case IsValidResult::SWITCH_TO_SELF: return "Cannot switch to self";
    case IsValidResult::SWITCH_ACTIVE_POKEMON: return "Cannot switch to an already active teammate";
    case IsValidResult::SWITCH_POKEMON_DEAD: return "Cannot switch to a dead pokemon";
    case IsValidResult::SWITCH_MUST_WAIT: return "Must wait for opponent's free move";
    case IsValidResult::SWITCH_LOCKED_BY_SCRIPT: return "Switch locked by script";
    case IsValidResult::WAIT_NOT_ALLOWED: return "Wait is not allowed unless opponent has a free move";
    case IsValidResult::STRUGGLE_NOT_ALLOWED: return "Struggle is not a valid action";
    case IsValidResult::ACTION_TYPE_DISABLED: return "Action type disabled";
    case IsValidResult::INVALID_FRIENDLY_TARGET: return "Invalid friendly target";
    default: return "Unknown invalid action reason";
      // clang-format on
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
  SEEDED,                  /**< Initial environment has been seeded. */
  SELECTORDER,             /**< Determine acter order and disambiguate ties. */
  PRETURN,                 /**< Before a Pokemon takes its turn. */
  // switch evaluation:
  PRESWITCH,               /**< Before a Pokemon switches out. */
  POSTSWITCH,              /**< After a Pokemon switches in. */
  // pre move evaluation:
  STATUS,                  /**< Before a move is executed, for status effects like paralysis. */
  // move damage evaluation:
  MOVEBASE,                /**< Base move evaluation. */
  EVALUATEHITCHANCE,       /**< Evaluate if the move hits. */
  DAMAGINGMOVEBASE,        /**< Base move evaluation for damaging moves. */
  EVALUATECRITCHANCE,      /**< Evaluate if the move crits. */
  SETBASEPOWER,            /**< Set the base power of the move. */
  SETMOVETYPE,             /**< Set the type of the move. */
  MODIFYBASEPOWER,         /**< Modify the base power of the move. */
  MODIFYATTACKPOWER,       /**< Modify the attack power of the user. */
  MODIFYCRITICALPOWER,     /**< Modify the power of a critical hit. */
  MODIFYRAWDAMAGE,         /**< Modify the raw damage calculated. */
  MODIFYSTAB,              /**< Modify the Same-Type Attack Bonus (STAB). */
  MODIFYTYPERESISTANCE,    /**< Modify the type resistance of the target. */
  MODIFYITEMPOWER,         /**< Modify the power based on items. */
  PREDAMAGE,               /**< Before damage is applied. */
  POSTDAMAGE,              /**< After damage is applied. */
  // post move evaluation:
  POSTMOVE,                /**< After a move has been executed. */
  PRESECONDARY,            /**< Before secondary effects are calculated. */
  EVALSECONDARYHITCHANCE,  /**< Evaluate if the secondary effect hits. */
  SECONDARY,               /**< Secondary effects are being applied. */
  // post turn status
  POSTTURN,                /**< After a Pokemon has completed its turn. */
  // post round status
  POSTROUND,               /**< After both Pokemon have completed their turns. */
  HASH,                    /**< The resulting environment is being hashed. */
  FINAL,                   /**< The final stage of the round. */
};

/**
 * @brief Converts a StageType to a human-readable string.
 * @param stage The StageType to convert.
 * @return A C-string representing the name of the stage.
 */
static const char* stageTypeToString(StageType stage) {
  switch (stage) {
    case StageType::DNE: return "DNE";
    case StageType::SEEDED: return "SEEDED";
    case StageType::SELECTORDER: return "SELECTORDER";
    case StageType::PRETURN: return "PRETURN";
    case StageType::PRESWITCH: return "PRESWITCH";
    case StageType::POSTSWITCH: return "POSTSWITCH";
    case StageType::STATUS: return "STATUS";
    case StageType::MOVEBASE: return "MOVEBASE";
    case StageType::EVALUATEHITCHANCE: return "EVALUATEHITCHANCE";
    case StageType::DAMAGINGMOVEBASE: return "DAMAGINGMOVEBASE";
    case StageType::EVALUATECRITCHANCE: return "EVALUATECRITCHANCE";
    case StageType::SETBASEPOWER: return "SETBASEPOWER";
    case StageType::SETMOVETYPE: return "SETMOVETYPE";
    case StageType::MODIFYBASEPOWER: return "MODIFYBASEPOWER";
    case StageType::MODIFYATTACKPOWER: return "MODIFYATTACKPOWER";
    case StageType::MODIFYCRITICALPOWER: return "MODIFYCRITICALPOWER";
    case StageType::MODIFYRAWDAMAGE: return "MODIFYRAWDAMAGE";
    case StageType::MODIFYSTAB: return "MODIFYSTAB";
    case StageType::MODIFYTYPERESISTANCE: return "MODIFYTYPERESISTANCE";
    case StageType::MODIFYITEMPOWER: return "MODIFYITEMPOWER";
    case StageType::PREDAMAGE: return "PREDAMAGE";
    case StageType::POSTDAMAGE: return "POSTDAMAGE";
    case StageType::POSTMOVE: return "POSTMOVE";
    case StageType::PRESECONDARY: return "PRESECONDARY";
    case StageType::EVALSECONDARYHITCHANCE: return "EVALSECONDARYHITCHANCE";
    case StageType::SECONDARY: return "SECONDARY";
    case StageType::POSTTURN: return "POSTTURN";
    case StageType::POSTROUND: return "POSTROUND";
    case StageType::HASH: return "HASH";
    case StageType::FINAL: return "FINAL";
    default: return "UNKNOWN";
  }
}

/** @} */

// clang-format on


/**
 * @def CALLPLUGIN
 * @brief A macro for invoking plugins of a specific type.
 *
 * This macro iterates through all registered plugins of a given `pluginType`
 * for the current matchup and calls their `pluginFunction`. The return value
 * of each plugin is OR'd with `retValue`. The loop breaks if `retValue`
 * becomes greater than 1, which is a convention to indicate that a plugin has
 * handled the event and no further plugins should be called.
 *
 * @param retValue The variable to store the combined return values of the
 * plugins.
 * @param pluginType The type of plugin to call (e.g., `PLUGIN_ON_MODIFYSPEED`).
 * @param pluginFunction The function signature of the plugin to be called.
 * @param ... The arguments to pass to the plugin function.
 */
#define CALLPLUGIN(retValue, pluginType, pluginFunction, ...)        \
  {                                                                  \
    const std::vector<plugin_t>& cPlugins =                          \
        getCPluginSet()[(size_t)pluginType];                         \
    for (auto iPlugin = cPlugins.cbegin(), iPSize = cPlugins.cend(); \
         iPlugin != iPSize;                                          \
         ++iPlugin) {                                                \
      pluginFunction cPlugin = (pluginFunction)iPlugin->pFunction;   \
      retValue = retValue | cPlugin(__VA_ARGS__);                    \
      if (retValue > 1) { break; }                                   \
    }                                                                \
  }


using PluginSet = std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>;
using PluginSets = std::array< std::array<PluginSet, 6>, 12>;
using ValidMoveSet = std::bitset<VALID_MOVE_SIZE>;
using ValidSwapSet = std::bitset<VALID_SWAP_SIZE>;

#endif // PKCU_TYPES_H
