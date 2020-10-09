#ifndef PKAI_GAME_H
#define	PKAI_GAME_H

#include "pkai.h"

#include <array>
#include <memory>
#include <vector>
#include <stdint.h>
#include <boost/program_options.hpp>

#include "action.h"
#include "environment_nonvolatile.h"
#include "environment_possible.h"
#include "environment_volatile.h"
#include "pkCU.h"
#include "planner.h"
#include "evaluator.h"


struct Turn {
  struct PerTeam {
    uint64_t numNodesEvaluated = 0;
    double timeSpent = 0;
    double simpleFitness = 0;
    double depth0Fitness = 0;
    double depthMaxFitness = 0;
    size_t activePokemon = 0;
    Action action;
  };

  EnvironmentPossibleData env;
  std::array<PerTeam, 2> teams;
  size_t stateSelected = SIZE_MAX;
  bool freeTurn = false;
};

struct GameResult {
  struct PerTeam {
    struct PerPokemon {
      std::array<double, 5> moveUse = {0., 0., 0., 0., 0.};
      double participation = 0;
      double aggregateContribution = 0;
      double simpleContribution = 0;
      double d0Contribution = 0;
      double dMaxContribution = 0;
      size_t ranking = 7;
    };

    std::array<PerPokemon, 6> pokemon;
    double lastSimpleFitness = 0;
    double timeSpent = 0;
    uint64_t numNodesEvaluated = 0;
  };

  std::vector<Turn> log;
  std::array<PerTeam, 2> teams;

  size_t numPlies = 0;
  int endStatus = MATCH_UNPLAYED;

  bool isPlayed() const { return endStatus != MATCH_UNPLAYED; }
};

struct HeatResult {
  struct PerTeam {
    struct PerPokemon {
      std::array<double, 5> moveUse = {0., 0., 0., 0., 0.};
      double participation = 0;
      double aggregateContribution = 0;
      double simpleContribution = 0;
      size_t ranking = 7;
    };

    std::array<PerPokemon, 6> pokemon;
    double lastSimpleFitness = 0;
    double averageTimeSpent = 0;
    uint64_t averageNodesEvaluated = 0;
  };

  std::vector<GameResult> gameResults;
  std::array<PerTeam, 2> teams;
  std::array<uint32_t, 2> score;
  uint64_t numPlies = 0.;
  int endStatus = MATCH_UNPLAYED;
  size_t matchesPlayed = 0;
  size_t matchesTotal = 0;

  double averagePlies() const { return double(numPlies) / double(matchesPlayed); }
};

class Game {
public:
  struct Config {
    /*
     * verbosity level, controls status printing.
     * 0: nothing is printed.
     * 1: terminal heat events are printed.
     * 2: terminal game events are printed.
     * 3: game state transition events are printed.
     */
    int verbosity = 0;

    /* maximum number of turns allowed before a draw occurs */
    size_t maxPlies = 120;

    /* number of matches, in best of N format, that game is to play before returning. Should ideally be an odd number */
    size_t maxMatches = 3;

    /* When above 0, n threads are instantiated for each match when performing heat rollouts. */
    size_t numThreads = 0;

    /* when true, manual state selection is used. */
    bool allowStateSelection = false;

    /* when true, undefined agents are replaced with a default agent */
    bool allowUndefinedAgents = true;

    /* when true, a HeatResult will return its component GameResults, and so on for Turns. Increases memory usage. */
    bool storeSubcomponents = true;

    Config(){};

    boost::program_options::options_description options(
        const std::string& category="game configuration",
        std::string prefix = "");
  };

  Game(const Config& cfg=Config());
  Game(const Game& other) = default;
  ~Game() {};

  /* create all variables, prepare game for running */
  Game& initialize();

  /* release all planners, engines, evaluators, and nonvolatile states */
  Game& clear();

  HeatResult rollout(const EnvironmentVolatileData& initialState) const;
  HeatResult rollout() const { return rollout(initialState_); }

  GameResult rollout_game(const EnvironmentVolatileData& initialState, size_t iMatch=SIZE_MAX) const;
  GameResult rollout_game(size_t iMatch=SIZE_MAX) const {
    return rollout_game(initialState_, iMatch);
  }
  
  /* main loop. Calls computation in pkai_cu, search via planner, input from user if necessary */
  HeatResult run() { 
    if (!isInitialized_) { initialize(); }
    return rollout();
  }

  Game& setEvaluator(const std::shared_ptr<Evaluator>& eval);
  Game& setEvaluator(const Evaluator& eval) {
    return setEvaluator(std::shared_ptr<Evaluator>(eval.clone()));
  }
  Game& setPlanner(size_t iAgent, const std::shared_ptr<Planner>& planner);
  Game& setPlanner(size_t iAgent, const Planner& planner) {
    return setPlanner(iAgent, std::shared_ptr<Planner>(planner.clone()));
  }
  Game& setInitialState(const EnvironmentVolatileData& initialState) {
    initialState_ = initialState;
    return *this;
  }
  Game& setTeam(size_t iAgent, const TeamNonVolatile& nv);
  Game& setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv);
  Game& setEnvironment(const EnvironmentNonvolatile& nv) {
    return setEnvironment(std::make_shared<EnvironmentNonvolatile>(nv));
  }
  Game& setEngine(const std::shared_ptr<PkCU>& cu);
  Game& setEngine(const PkCU& cu) {
    return setEngine(std::make_shared<PkCU>(cu));
  }

  Game& setAllowStateSelection(bool allow) { cfg_.allowStateSelection = allow; return *this; };
  Game& setMaxMatches(size_t maxMatches) {
    if ((maxMatches & 1) == 0) { maxMatches += 1; }
    cfg_.maxMatches = maxMatches;
    return *this;
  };

  Game& setMaxPlies(size_t maxPlies) { cfg_.maxPlies = maxPlies; return *this; }
  Game& setVerbosity(int verbosity) { cfg_.verbosity = verbosity; return *this; }

protected:
  Config cfg_;

  /* planner class for players */
  std::array<std::shared_ptr<Planner>, 2> agents_;

  /* state transition engine, given an environment_nonvolatile provides state transitions of an environment_volatile */
  std::shared_ptr<PkCU> cu_;

  std::shared_ptr<const EnvironmentNonvolatile> nv_;

  std::shared_ptr<Evaluator> eval_;

  EnvironmentVolatileData initialState_;

  bool isInitialized_;

  /* create a log of the current turn */
  Turn digestTurn(
      const std::array<PlannerResult, 2>& actions,
      size_t resultingState,
      const ConstEnvironmentPossible& envP) const;

  /* creates a log of the current completed game */
  GameResult digestGame(
      std::vector<Turn>& cLog, const ConstEnvironmentVolatile& initialState, int gameResult) const;

  HeatResult digestMatch(std::vector<GameResult>& gLog) const;

  std::string getPokemonIdentifier(const ConstTeamVolatile& cTeam, size_t iTeam) const;
  std::string getGameIdentifier(size_t iMatch) const;
  std::string getTeamIdentifier(size_t iTeam) const;

  void incrementScore(int matchResult, std::array<uint32_t, 2>& score) const;

  /* prints the current action */
  void printAction(
      const ConstTeamVolatile& currentTeam, const Action& indexAction, unsigned int iTeam) const;

  void printStateTransition(const Turn& cTurn, size_t iPly=SIZE_MAX) const;

  /* prints interesting facts about the game */
  void printGameStart(size_t iMatch=SIZE_MAX) const;
  void printGameOutline(const GameResult& gResult, size_t iMatch=SIZE_MAX) const;

  /* print interesting facts about the heat */
  void printHeatStart() const;
  void printHeatOutline(const HeatResult& hResult) const;
};

#endif	/* PKAI_GAME_H */

