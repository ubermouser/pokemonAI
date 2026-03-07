/**
 * @file neo_pkCU.h
 * @brief Defines the new battle engine (NeoPkCU) and its internal state machine
 * (NeoPkCUEngine) with stub implementations.
 */
#ifndef NEO_PKAI_CU_H
#define NEO_PKAI_CU_H

#include "pkai.h"
#include <memory>
#include <vector>
#include <boost/program_options.hpp>
#include "engine.h"
#include "legacy_pkCU.h" // For shared types like PossibleEnvironments, Action, etc.

class NeoPkCUEngine;

class PKAISHARED NeoPkCU {
public:
  struct Config {
    size_t numRandomEnvironments = 1;
    bool allowInvalidMoves = false;
    Config(){};
    boost::program_options::options_description options(
        const std::string& category="engine configuration",
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

  static bool isMoveAction(const Action& action);
  static bool isSwitchAction(const Action& action);

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
