#ifndef PKAI_GAME_H
#define	PKAI_GAME_H

#include "../inc/pkai.h"

#include <vector>
#include <array>
#include <stdint.h>

#include "../inc/environment_nonvolatile.h"
#include "../inc/environment_possible.h"
#include "../inc/environment_volatile.h"

class Planner;
class PkCU;
class TeamNonVolatile;
class Evaluator;

union EnvironmentPossible;
union EnvironmentVolatile;
union TeamVolatile;

struct Turn
{
  EnvironmentVolatile env;
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

class Game
{
private:
  /* planner class for players */
  std::array<Planner*, 2> agents;

  /* record of pokemon moves, fitness result, etc */
  std::vector<std::vector<Turn> > gameLog;

  /* record of pokemon participation, statistics about the match */
  std::vector<GameResult> gameResults;

  HeatResult hResult;

  /* templated state transition engine, given an environment_nonvolatile provides state transitions of an environment_volatile */
  PkCU* cu;

  EnvironmentNonvolatile cEnvNV;

  /* maximum number of turns allowed before a draw occurs */
  size_t maxPlies;

  /* number of matches, in best of N format, that game is to play before returning. Should ideally be an odd number */
  size_t maxMatches;

  size_t gameAccuracy;

  bool rollout;

  bool allowStateSelection;

  bool isInitialized;

  EnvironmentPossible initialEnvP;

  EnvironmentPossible cEnvP;

  /* create a log of the current turn */
  void digestTurn(std::vector<Turn>& cLog, unsigned int actionTeamA, unsigned int actionTeamB, size_t resultingState,  const EnvironmentPossible& envP);

  /* creates a log of the current completed game */
  void digestGame(const std::vector<Turn>& cLog, int gameResult );

  void digestMatch(const std::vector<GameResult>& gLog, const std::array<unsigned int, 2>& numWins, int matchResult );
  
  /* Returns a valid action as per the user's choice 
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   */
  unsigned int actionSelect(const EnvironmentVolatile& env, unsigned int iTeam);
  
  /* Selects a state as per the user's choice to evaluate upon */
  const EnvironmentVolatile& stateSelect(const std::vector<EnvironmentPossible>& possibleEnvironments);

  /* Selects a state as per the user's choice to evaluate upon */
  size_t stateSelect_index(const std::vector<EnvironmentPossible>& possibleEnvironments);
  
  /* selects a state at random, giving greater odds to state with higher probabilities of occurance */
  size_t stateSelect_Roulette(const std::vector<EnvironmentPossible>& possibleEnvironments);

  /* Prints the pokemon of a given team to stdout 
   * @param verbosity - how verbose should the output of the printed teams be?
   *	0 - print name of pokemon only, followed by newline
   *	1 - print index of pokemon followed by name of pokemon, then newline
   *	2 - index, name, species
   *	3 - index, name, species, hitpoints / (boosted)total, status effects
   */
  void printTeam(const TeamVolatile& currentTeam, unsigned int iTeam, unsigned int verbosity);

  /*
   * Prints all possible actions a given pokemon may take to stdout
   */
  void printActions(const EnvironmentVolatile& cEnv, unsigned int iTeam);

  /* prints the current action */
  void printAction(const TeamVolatile& currentTeam, unsigned int indexAction, unsigned int iTeam);
  
  /* Print details of all possible states */
  void printStates(const std::vector<EnvironmentPossible>& possibleEnvironments, size_t numUnique);

  /* print details of a single state */
  void printState(const EnvironmentPossible& possible, size_t iState, size_t iPly = SIZE_MAX);

  /* prints interesting facts about the game */
  void printGameOutline(const GameResult& gResult, const std::vector<Turn>& gLog, const EnvironmentNonvolatile& cEnv);

  /* print interesting facts about the heat */
  void printMatchOutline(const EnvironmentNonvolatile& cEnv);

  /* Prints all loaded teams to stdout 
   * @param verbosity - how verbose should the output of the printed teams be?
   *	0 - print name of teams only, followed by newline
   *	1 - print index of team followed by name of team, followed by newline
   *	2 - print index of team followed by name of team, along with the 
   *		name and species of the pokemon
   */
  void printTeams(unsigned int verbosity);

public:
  Game(size_t _maxPlies = 100, size_t _maxMatches = 1, size_t _gameAccuracy = 1, bool rollout = false);

  ~Game();

  /* destroy memory intensive objects, but do not destroy game */
  void cleanUp();

  /* create all variables, prepare game for running */
  bool initialize();
  
  /* main loop. Calls computation in pkai_cu, search via planner, input from user if necessary */
  void run();

  void setTeam(size_t iAgent, const TeamNonVolatile& cTeam);

  void setEvaluator(size_t iAgent, const Evaluator& eval);

  void setPlanner(size_t iAgent, const Planner& cPlanner);

  const Planner* getPlanner(size_t iAgent) const;

  void setInitialState(const EnvironmentVolatile& rolloutState);

  void setEnvironment(const EnvironmentNonvolatile& _envNV);

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

  const EnvironmentNonvolatile& getEnvNV() const;
};

#endif	/* PKAI_GAME_H */

