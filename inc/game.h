#ifndef PKAI_GAME_H
#define	PKAI_GAME_H

#include "pkai.h"

#include <array>
#include <memory>
#include <vector>
#include <stdint.h>

#include "environment_nonvolatile.h"
#include "environment_possible.h"
#include "environment_volatile.h"
#include "pkCU.h"
#include "planner.h"
#include "evaluator.h"


struct Turn
{
  EnvironmentVolatileData env;
  std::array<uint32_t, 2> activePokemon;
  std::array<uint32_t, 2> action;
  std::array<uint32_t, 2> prediction;
  std::array<fpType, 2> simpleFitness;
  std::array<fpType, 2> depth0Fitness;
  std::array<fpType, 2> depthMaxFitness;
  fpType probability;
  uint32_t stateSelected;
};

struct GameResult
{
  std::array<std::array<std::array<uint32_t, 5>, 6>, 2> moveUse; 
  std::array<std::array<uint32_t, 6>, 2> participation;
  std::array<std::array<fpType, 6>, 2> aggregateContribution;
  std::array<std::array<fpType, 6>, 2> simpleContribution;
  std::array<std::array<fpType, 6>, 2> d0Contribution;
  std::array<std::array<fpType, 6>, 2> dMaxContribution;
  std::array<std::array<uint32_t, 6>, 2> ranking;
  std::array<fpType, 2> predictionAccuracy;
  uint32_t numPlies;
  int endStatus;
};

struct HeatResult
{
  std::array<std::array<fpType, 6>, 2> participation;
  std::array<std::array<fpType, 6>, 2> aggregateContribution;
  std::array<std::array<uint32_t, 6>, 2> ranking;
  std::array<uint32_t, 2> score;
  std::array<fpType, 2> predictionAccuracy;
  fpType numPlies;
  int endStatus;
  size_t matchesPlayed;
  size_t matchesTotal;
};

class Game {
protected:
  /* planner class for players */
  std::array<std::shared_ptr<Planner>, 2> agents;

  /* record of pokemon moves, fitness result, etc */
  std::vector<std::vector<Turn> > gameLog;

  /* record of pokemon participation, statistics about the match */
  std::vector<GameResult> gameResults;

  HeatResult hResult;

  /* templated state transition engine, given an environment_nonvolatile provides state transitions of an environment_volatile */
  std::shared_ptr<PkCU> cu_;

  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  std::shared_ptr<Evaluator> eval_;

  /* maximum number of turns allowed before a draw occurs */
  size_t maxPlies;

  /* number of matches, in best of N format, that game is to play before returning. Should ideally be an odd number */
  size_t maxMatches;

  bool rollout;

  bool allowStateSelection;

  bool isInitialized;

  EnvironmentVolatileData initialState_;

  /* create a log of the current turn */
  void digestTurn(
      std::vector<Turn>& cLog,
      unsigned int actionTeamA,
      unsigned int actionTeamB,
      size_t resultingState,  const ConstEnvironmentPossible& envP);

  /* creates a log of the current completed game */
  void digestGame(const std::vector<Turn>& cLog, int gameResult );

  void digestMatch(
      const std::vector<GameResult>& gLog,
      const std::array<unsigned int, 2>& numWins,
      int matchResult);

  /* prints the current action */
  void printAction(
      const ConstTeamVolatile& currentTeam, unsigned int indexAction, unsigned int iTeam);

  /* prints interesting facts about the game */
  void printGameOutline(
      const GameResult& gResult, const std::vector<Turn>& gLog, const EnvironmentNonvolatile& cEnv);

  /* print interesting facts about the heat */
  void printMatchOutline(const EnvironmentNonvolatile& cEnv);

public:
  Game(size_t _maxPlies = 100, size_t _maxMatches = 1, bool rollout = false);

  ~Game() {};

  /* destroy memory intensive objects, but do not destroy game */
  void cleanUp();

  /* create all variables, prepare game for running */
  bool initialize();
  
  /* main loop. Calls computation in pkai_cu, search via planner, input from user if necessary */
  void run();

  void setTeam(size_t iAgent, const TeamNonVolatile& cTeam);

  void setEvaluator(size_t iAgent, const std::shared_ptr<Evaluator>& eval);
  void setDigestEvaluator(const std::shared_ptr<Evaluator>& eval);
  void setPlanner(size_t iAgent, const std::shared_ptr<Planner>& cPlanner);
  void setInitialState(const EnvironmentVolatileData& rolloutState);
  void setEnvironment(const EnvironmentNonvolatile& _envNV);
  void setEngine(const PkCU& cu);

  void setAllowStateSelection(bool allow) { allowStateSelection = allow; };

  void setMaxMatches(size_t _maxMatches)
  {
    if ((_maxMatches & 1) == 0) { _maxMatches += 1; }
    maxMatches = _maxMatches;

    isInitialized = false;
  };

  void setMaxPlies(size_t _maxPlies)
  {
    maxPlies = _maxPlies;

    isInitialized = false;
  }

  const HeatResult& getResult() const;

  const std::vector<GameResult>& getGameResults() const;

  const std::vector<Turn>& getGameLog(size_t iGame) const;
};

#endif	/* PKAI_GAME_H */

