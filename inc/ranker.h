#ifndef RANKER_H
#define RANKER_H

#include "pkai.h"

#include <stdint.h>
#include <array>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include "game.h"
#include "game_factory.h"
#include "pkCU.h"
#include "true_skill.h"
#include "ranked.h"

#include "league.h"
#include "ranked_battlegroup.h"
#include "ranked_team.h"
#include "ranked_pokemon.h"
#include "ranked_evaluator.h"
#include "ranked_planner.h"


struct GameHeat {
  BattlegroupPtr team_a;
  
  BattlegroupPtr team_b;

  double matchQuality;

  HeatResult heatResult;
};


struct LeagueHeat : public League {
  std::vector<GameHeat> games;

  LeagueHeat(const League& league = League{}) : League(league) {}
};

class Ranker {
public:
  struct Config {
    /*verbosity of trainer */
    int verbosity = 0;

    /* random seed of the trainer */
    uint32_t randomSeed = 0;

    /*when above 0, thread parallelism is used to invoke games*/
    size_t numThreads = 0;

    /* when choosing a possible match, consider at most this many pairs */
    size_t maxMatchesToConsider = 1000;

    /* The maximum number of elements in a leaderboard to print*/
    size_t leaderboardPrintCount = 20;

    /* the minimum number of games per generation */
    size_t minGamesPerBattlegroup = 10;

    /* if a team population is to be loaded to memory from a directory, this is where it is */
    std::string teamPath = "teams";

    /* minimum amount of time to work on a given league, in seconds */
    double minimumWorkTime = 20;

    Battlegroup::Contribution contributions;

    bool saveOnCompletion = false;

    bool printPokemonLeaderboard = true;

    bool printBattlegroupLeaderboard = true;

    /* do we allow teams to rank against teams of different leagues? Useful for small population */
    bool enforceSameLeague = false;

    /* when true, matches against Battlegroups with the same nonvolatile team / evaluator / planner may take place */
    bool allowSameTeam = false;

    bool allowSameEvaluator = true;

    bool allowSamePlanner = true;

    boost::program_options::options_description options(
        const std::string& category="trainer configuration",
        std::string prefix = "");

    Config() {};
  };

  struct RankerThread {
    /* game instance used for evaluation, per thread */
    std::shared_ptr<Game> game;

    /* engine instance used for state evaluation, per thread */
    std::shared_ptr<PkCU> engine;

    /* state evaluator used per engine, per thread */
    std::shared_ptr<Evaluator> stateEvaluator;
  };

  Ranker(const Config& cfg = Config{});
  virtual ~Ranker() {};

  /* create all variables, prepare trainer for running */
  virtual void initialize();

  LeagueHeat rank() const;

  /* plays minGamesPerGeneration games for every tsTeam within league */
  void runLeague(LeagueHeat& league) const;

  /* plays minGamesPerGeneration games with tsTeam within league*/
  void gauntlet(BattlegroupPtr& tsTeam, LeagueHeat& league) const;

  Ranker& setGameFactory(const GameFactory& gameFactory) { gameFactory_ = gameFactory; return *this; }

  Ranker& setGame(const Game& game);
  Ranker& setEngine(const PkCU& cu);
  Ranker& setStateEvaluator(const Evaluator& eval);

  Ranker& addEvaluator(const std::shared_ptr<Evaluator>& evaluator);
  Ranker& addEvaluator(const Evaluator& evaluator) {
    return addEvaluator(std::shared_ptr<Evaluator>(evaluator.clone()));
  }

  Ranker& addPlanner(const std::shared_ptr<Planner>& planner);
  Ranker& addPlanner(const Planner& planner) {
    return addPlanner(std::shared_ptr<Planner>(planner.clone()));
  }

  Ranker& addTeam(const TeamNonVolatile& cTeam);
  
protected:
  Config cfg_;

  GameFactory gameFactory_;

  /* population of teams to be run on. 0 implies no work will be done on the league */
  League initialLeague_;

  std::reference_wrapper<std::ostream> out_;

  /* per-thread data */
  mutable std::vector<RankerThread> threads_;

  mutable std::default_random_engine rand_;

  bool initialized_;

  GameHeat singleGame(
      BattlegroupPtr& team_a,
      BattlegroupPtr& team_b) const;

  /* stochastically find a match of ideal skill for a given team. If enforceSameLeague is false, allow matches from nearby leagues */
  BattlegroupPtr findMatch(const Battlegroup& oTeam, const LeagueHeat& league) const;
  BattlegroupPtr findSubsampledMatch(const Battlegroup& oTeam, const BattlegroupLeague& leauge) const;

  /* calculates interesting things about the given league */
  void calculateDescriptiveStatistics(const LeagueHeat& league) const;

  /* print information about the top n members of league iLeague */
  void printLeagueStatistics(LeagueHeat& league) const;

  /* print information about a single heat */
  void printHeatResult(const GameHeat& heat) const;
  
  /*load a population of pokemon and their rankings from a filepath */
  size_t loadTeamPopulation();

  size_t saveTeamPopulation(const League& league) const;

  virtual LeagueHeat constructLeague() const;

  //BattlegroupLeague constructBattlegroups(const League& league) const;

  void digestGame(GameHeat& gameHeat, LeagueHeat& league) const;

  void testInitialized() const;
};

#endif /* RANKER_H */