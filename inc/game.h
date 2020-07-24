#ifndef PKAI_GAME_H
#define	PKAI_GAME_H

#include "../inc/pkai.h"

#include <vector>
#include <boost/array.hpp>
#include <stdint.h>

#include "../inc/environment_nonvolatile.h"
#include "../inc/environment_possible.h"
#include "../inc/environment_volatile.h"

class planner;
class pkCU;
class team_nonvolatile;
class evaluator;

union environment_possible;
union environment_volatile;
union team_volatile;

struct turn
{
  environment_volatile env;
  boost::array<uint32_t, 2> activePokemon;
  boost::array<uint32_t, 2> action;
  boost::array<uint32_t, 2> prediction;
  boost::array<fpType, 2> simpleFitness;
  boost::array<fpType, 2> depth0Fitness;
  boost::array<fpType, 2> depthMaxFitness;
  fpType probability;
  uint32_t stateSelected;
};

struct gameResult
{
  boost::array<boost::array<boost::array<uint32_t, 5>, 6>, 2> moveUse; 
  boost::array<boost::array<uint32_t, 6>, 2> participation;
  boost::array<boost::array<fpType, 6>, 2> aggregateContribution;
  boost::array<boost::array<fpType, 6>, 2> simpleContribution;
  boost::array<boost::array<fpType, 6>, 2> d0Contribution;
  boost::array<boost::array<fpType, 6>, 2> dMaxContribution;
  boost::array<boost::array<uint32_t, 6>, 2> ranking;
  boost::array<fpType, 2> predictionAccuracy;
  uint32_t numPlies;
  int endStatus;
};

struct heatResult
{
  boost::array<boost::array<fpType, 6>, 2> participation;
  boost::array<boost::array<fpType, 6>, 2> aggregateContribution;
  boost::array<boost::array<uint32_t, 6>, 2> ranking;
  boost::array<uint32_t, 2> score;
  boost::array<fpType, 2> predictionAccuracy;
  fpType numPlies;
  int endStatus;
  size_t matchesPlayed;
  size_t matchesTotal;
};

class game
{
private:
  /* planner class for players */
  boost::array<planner*, 2> agents;

  /* record of pokemon moves, fitness result, etc */
  std::vector<std::vector<turn> > gameLog;

  /* record of pokemon participation, statistics about the match */
  std::vector<gameResult> gameResults;

  heatResult hResult;

  /* templated state transition engine, given an environment_nonvolatile provides state transitions of an environment_volatile */
  pkCU* cu;

  environment_nonvolatile cEnvNV;

  /* maximum number of turns allowed before a draw occurs */
  size_t maxPlies;

  /* number of matches, in best of N format, that game is to play before returning. Should ideally be an odd number */
  size_t maxMatches;

  size_t gameAccuracy;

  bool rollout;

  bool allowStateSelection;

  bool isInitialized;

  environment_possible initialEnvP;

  environment_possible cEnvP;

  /* create a log of the current turn */
  void digestTurn(std::vector<turn>& cLog, unsigned int actionTeamA, unsigned int actionTeamB, size_t resultingState,  const environment_possible& envP);

  /* creates a log of the current completed game */
  void digestGame(const std::vector<turn>& cLog, int gameResult );

  void digestMatch(const std::vector<gameResult>& gLog, const boost::array<unsigned int, 2>& numWins, int matchResult );
  
  /* Returns a valid action as per the user's choice 
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   */
  unsigned int actionSelect(const environment_volatile& env, unsigned int iTeam);
  
  /* Selects a state as per the user's choice to evaluate upon */
  const environment_volatile& stateSelect(const std::vector<environment_possible>& possibleEnvironments);

  /* Selects a state as per the user's choice to evaluate upon */
  size_t stateSelect_index(const std::vector<environment_possible>& possibleEnvironments);
  
  /* selects a state at random, giving greater odds to state with higher probabilities of occurance */
  size_t stateSelect_Roulette(const std::vector<environment_possible>& possibleEnvironments);

  /* Prints the pokemon of a given team to stdout 
   * @param verbosity - how verbose should the output of the printed teams be?
   *	0 - print name of pokemon only, followed by newline
   *	1 - print index of pokemon followed by name of pokemon, then newline
   *	2 - index, name, species
   *	3 - index, name, species, hitpoints / (boosted)total, status effects
   */
  void printTeam(const team_volatile& currentTeam, unsigned int iTeam, unsigned int verbosity);

  /*
   * Prints all possible actions a given pokemon may take to stdout
   */
  void printActions(const environment_volatile& cEnv, unsigned int iTeam);

  /* prints the current action */
  void printAction(const team_volatile& currentTeam, unsigned int indexAction, unsigned int iTeam);
  
  /* Print details of all possible states */
  void printStates(const std::vector<environment_possible>& possibleEnvironments, size_t numUnique);

  /* print details of a single state */
  void printState(const environment_possible& possible, size_t iState, size_t iPly = SIZE_MAX);

  /* prints interesting facts about the game */
  void printGameOutline(const gameResult& gResult, const std::vector<turn>& gLog, const environment_nonvolatile& cEnv);

  /* print interesting facts about the heat */
  void printMatchOutline(const environment_nonvolatile& cEnv);

  /* Prints all loaded teams to stdout 
   * @param verbosity - how verbose should the output of the printed teams be?
   *	0 - print name of teams only, followed by newline
   *	1 - print index of team followed by name of team, followed by newline
   *	2 - print index of team followed by name of team, along with the 
   *		name and species of the pokemon
   */
  void printTeams(unsigned int verbosity);

public:
  game(size_t _maxPlies = 100, size_t _maxMatches = 1, size_t _gameAccuracy = 1, bool rollout = false);

  ~game();

  /* destroy memory intensive objects, but do not destroy game */
  void cleanUp();

  /* create all variables, prepare game for running */
  bool initialize();
  
  /* main loop. Calls computation in pkai_cu, search via planner, input from user if necessary */
  void run();

  void setTeam(size_t iAgent, const team_nonvolatile& cTeam);

  void setEvaluator(size_t iAgent, const evaluator& eval);

  void setPlanner(size_t iAgent, const planner& cPlanner);

  const planner* getPlanner(size_t iAgent) const;

  void setInitialState(const environment_volatile& rolloutState);

  void setEnvironment(const environment_nonvolatile& _envNV);

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

  const heatResult& getResult() const;

  const std::vector<gameResult>& getGameResults() const;

  const std::vector<turn>& getGameLog(size_t iGame) const;

  const environment_nonvolatile& getEnvNV() const;
};

#endif	/* PKAI_GAME_H */

