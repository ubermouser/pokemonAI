/**
 * @file neo_pkCU.h
 * @brief Defines the new battle engine (NeoPkCU) and its internal state machine
 * (NeoPkCUEngine) with stub implementations.
 */
#ifndef NEO_PKAI_CU_H
#define NEO_PKAI_CU_H

#include <boost/program_options.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/join.hpp>
#include <memory>
#include <vector>

#include "engine.h"
#include "neo_pkCU_engine.h"
#include "pkCU_types.h"
#include "pkai.h"


class NeoPkCUEngine;

class PKAISHARED NeoPkCU {
 public:
  /**
   * @enum StateSelectMethod
   * @brief Defines how the engine selects resulting environments.
   */
  enum class StateSelectMethod {
    RANDOM,      /**< Select a single state at random (roulette). */
    MOST_LIKELY, /**< Select the single most probable state. */
    ALL          /**< Return all possible states. */
  };

  /**
   * @struct Config
   * @brief Configuration options for the NeoPkCU engine.
   */
  struct Config {
    /*
     * verbosity level, controls status printing.
     * 0: all engine transitions are printed.
     * 1: transition results are printed.
     * 2: nothing is printed.
     */
    int verbosity = 2;

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
     * @brief The number of active Pokemon on each team in the battle.
     *
     * This controls the number of Pokemon that can be active in the battle.
     * Upon match start, the first N pokemon in each team are set to be active.
     */
    size_t numActivePokemon = 1;

    /**
     * @brief The method used to select resulting environments.
     *
     * - RANDOM: returns a single environment chosen at random (roulette).
     * - MOST_LIKELY: returns the single most likely environment.
     * - ALL: returns all probabilistic environments.
     */
    StateSelectMethod stateSelectMethod = StateSelectMethod::ALL;


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

  /**
   * @brief Initializes the plugin sets for the configured non-volatile
   * environment.
   */
  void initialize();

  NeoPkCU& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& cEnv);
  NeoPkCU& setEnvironment(const EnvironmentNonvolatile& cEnv);

  NeoPkCU& setAccuracy(size_t engineAccuracy);
  NeoPkCU& setNumActivePokemon(size_t numActivePokemon);
  NeoPkCU& setStateSelectMethod(StateSelectMethod method);
  NeoPkCU& setAllowInvalidMoves(bool allow = true);

  /**
   * @brief Simulates a single turn of a Pokemon battle.
   *
   * Given the current volatile environment and the actions of both players,
   * this method calculates all possible resulting environments and their
   * probabilities. The results are returned as a `PossibleEnvironments`
   * object, which is a collection of `EnvironmentPossible` objects.
   *
   * @param cEnv The current volatile environment.
   * @param actionA The action of the first active pokemon of team A.
   * @param actionB The action of the first active pokemon of team B.
   * @return A `PossibleEnvironments` object containing all possible outcomes.
   */
  PossibleEnvironments updateState(
      const ConstEnvironmentVolatile& cEnv,
      const Action& actionA,
      const Action& actionB) const;
  PossibleEnvironments updateState(
      const ConstEnvironmentPossible& cEnvP,
      const Action& actionA,
      const Action& actionB) const;
  PossibleEnvironments updateState(
      const ConstEnvironmentVolatile& cEnv,
      const ActionMap& actionsA,
      const ActionMap& actionsB) const;
  PossibleEnvironments updateState(
      const ConstEnvironmentVolatile& cEnv, const ActionMap& actions) const;


  /**
   * @brief Returns the initial volatile state for the configured environment.
   * @return A `ConstEnvironmentVolatile` object representing the initial state.
   */
  ConstEnvironmentVolatile initialState() const;

  /**
   * @brief Returns a list of all valid actions for the first active pokemon of
   * a given team.
   * @param envV The current volatile environment.
   * @param iTeam The index of the team.
   * @return An `ActionVector` containing all valid actions.
   */
  ActionVector getValidActions(
      const ConstEnvironmentVolatile& envV, const Actor& actor) const;
  ActionVector getValidMoveActions(
      const ConstEnvironmentVolatile& envV, const Actor& actor) const;
  ActionVector getValidSwapActions(
      const ConstEnvironmentVolatile& envV, const Actor& actor) const;
  [[deprecated]] ActionVector getValidActions(
      const ConstEnvironmentVolatile& envV, TEAM iTeam) const;
  [[deprecated]] ActionVector getValidMoveActions(
      const ConstEnvironmentVolatile& envV, TEAM iTeam) const;
  [[deprecated]] ActionVector getValidSwapActions(
      const ConstEnvironmentVolatile& envV, TEAM iTeam) const;

  /**
   * @brief Checks if a given action is valid for a team in the current state.
   * @param envV The current volatile environment.
   * @param actor The teammate performing the action.
   * @param action The action to check.
   * @return An `IsValidResult` object indicating if the action is valid.
   */
  [[deprecated]] IsValidResult isValidAction(
      const ConstEnvironmentVolatile& envV,
      const TEAM iTeam,
      const Action& action) const;
  [[deprecated]] IsValidResult isValidAction(
      const ConstEnvironmentPossible& envV,
      const TEAM iTeam,
      const Action& action) const;
  IsValidResult isValidAction(
      const ConstEnvironmentVolatile& envV,
      const Actor& actor,
      const Action& action) const;
  IsValidResult isValidAction(
      const ConstEnvironmentPossible& envP,
      const Actor& actor,
      const Action& action) const;

 protected:
  IsValidResult isValidAction_move(
      const ConstEnvironmentVolatile& envV,
      const Actor& actor,
      const Action& action) const;
  IsValidResult isValidAction_switch(
      const ConstEnvironmentVolatile& envV,
      const Actor& actor,
      const Action& action) const;
  IsValidResult isValidAction_wait(
      const ConstEnvironmentVolatile& envV,
      const Actor& actor,
      const Action& action) const;
  IsValidResult isValidAction_struggle(
      const ConstEnvironmentVolatile& envV,
      const Actor& actor,
      const Action& action) const;

  ValidMoveSet getValidMoveFlags(
      const ConstEnvironmentVolatile& envV,
      const Actor& actor,
      const Action& action,
      const ConstPokemonVolatile& cPKV,
      const ConstMoveVolatile& cMV,
      const std::vector<Actor>& targets) const;

  ValidSwapSet getValidSwapFlags(
      const ConstEnvironmentVolatile& envV,
      const Actor& actor,
      const Action& action,
      const ConstPokemonVolatile& cPKV) const;

 public:
  std::vector<ActionMap> getAllValidActions(
      const ConstEnvironmentVolatile& envV, TEAM agentTeam = TEAM_A) const;


  /**
   * @brief Checks if the game is over.
   * @param envV The current environment.
   * @return `true` if the game is over, `false` otherwise.
   */
  bool isGameOver(const ConstEnvironmentPossible& envV) const;
  bool isGameOver(const ConstEnvironmentVolatile& envV) const;

  /**
   * @brief Returns the current state of the match.
   * @param envV The current environment.
   * @return A `MatchState` enum value indicating the game's status.
   */
  MatchState getGameState(const ConstEnvironmentVolatile& envV) const;
  MatchState getGameState(const ConstEnvironmentPossible& envV) const;

protected:
  Config cfg_;

  /**
   * @brief The non-volatile environment for the current battle.
   *
   * The PkCU instance loads plugins based on the teams in this environment.
   */
  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  /**
   * @brief The initial volatile state for the configured non-volatile
   * environment.
   */
  EnvironmentVolatileData initialState_;

  /**
   * @brief Whether the engine has been initialized for the current environment.
   */
  bool initialized_;


  /**
   * @brief All plugins loaded for the given nonvolatile state.
   */
  PluginSet pluginSet_;

  friend class NeoPkCUEngine;


  /**
   * @brief Throws an exception if the provided environment's non-volatile state
   * does not match the engine's configured state.
   * @param cEnv The environment to check.
   */
  void guardNonvolatileState(const ConstEnvironmentVolatile& cEnv) const;

  void guardCorrectActionCount(
      const ConstEnvironmentVolatile& cEnv, const ActionMap& actions) const;

  void guardInvalidActions(
      const ConstEnvironmentVolatile& cEnv, const ActionMap& actions) const;

  void guardInitialized() const;

  /**
   * @brief Creates the initial volatile state for the configured non-volatile
   * environment.
   */
  EnvironmentVolatileData createInitialVolatileState() const;
};


#endif /* NEO_PKAI_CU_H */
