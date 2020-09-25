#ifndef TRAINER_H
#define TRAINER_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <array>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/program_options.hpp>

#include "game.h"
#include "game_factory.h"
#include "true_skill.h"
#include "ranked.h"

#include "ranked_battlegroup.h"
#include "ranked_team.h"
#include "ranked_pokemon.h"
#include "ranked_evaluator.h"
#include "ranked_planner.h"


struct TrainerResult {
  // averages: (rank, mean, stdDev, plies, games, wins, draws)
  std::array<fpType, 7> averages;
  // stdDevs:
  std::array<fpType, 7> stdDevs;
  // minimums:
  std::array<fpType, 7> mins;
  // maximums:
  std::array<fpType, 7> maxes;
  // highest counts:
  const PokemonBase* highestPokemon;
  const Ability* highestAbility;
  const Item* highestItem;
  const Type* highestType;
  const Nature* highestNature;
  const Move* highestMove;
  size_t highestPokemonCount, highestAbilityCount, highestItemCount, highestTypeCount, highestNatureCount, highestMoveCount;
};

struct GameHeat {
  BattlegroupPtr team_a;
  
  BattlegroupPtr team_b;

  HeatResult heatResult;
};

struct LeagueHeat {
  PlannerLeague planners;

  EvaluatorLeague evaluators;

  TeamLeague teams;

  PokemonLeague pokemon;

  BattlegroupLeague battlegroups;

  std::vector<GameHeat> games;
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

    /* The maximum number of elements in a leaderboard to print*/
    size_t leaderboardPrintCount = 20;

    /* the minimum number of games per generation */
    size_t minGamesPerBattlegroup = 10;

    /* if a team population is to be loaded to memory from a directory, this is where it is */
    std::string teamPath = "teams";

    /* if value is nonzero, the number of generations between writeOuts to disk. Otherwise, do not write out */
    size_t writeOutEvery = 0;

    /* minimum amount of time to work on a given league, in seconds */
    double minimumWorkTime = 20;

    Game::Config game;

    Battlegroup::Contribution contributions;

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

  Ranker(const Config& cfg = Config{});
  virtual ~Ranker() {};

  /* create all variables, prepare trainer for running */
  void initialize();

  LeagueHeat rank() const;

  /* plays minGamesPerGeneration games for every tsTeam within league */
  void runLeague(LeagueHeat& league) const;

  /* plays minGamesPerGeneration games with tsTeam within league*/
  void gauntlet(BattlegroupPtr& tsTeam, LeagueHeat& league) const;

  Ranker& setGameFactory(const GameFactory& gameFactory) { gameFactory_ = gameFactory; return *this; }

  Ranker& setGame(const Game& game);

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
  TeamLeague teams_;

  PokemonLeague pokemon_;

  /* population of evaluators to be run on. If networks is 0, this MUST be greater than 0. */
  EvaluatorLeague evaluators_;

  PlannerLeague planners_;

  std::reference_wrapper<std::ostream> out_;

  /* game instance used for evaluation, per thread */
  mutable std::vector<Game> games_;

  mutable std::default_random_engine rand_;

  bool initialized_;

  GameHeat singleGame(
      BattlegroupPtr& team_a,
      BattlegroupPtr& team_b) const;

  /* stochastically find a match of ideal skill for a given team. If enforceSameLeague is false, allow matches from nearby leagues */
  BattlegroupPtr findMatch(const Battlegroup& oTeam, const LeagueHeat& league) const;

  /* calculates interesting things about the given league */
  void calculateDescriptiveStatistics(size_t iLeague, TrainerResult& cResult) const;

  /* print information about the top n members of league iLeague */
  void printLeagueStatistics(const LeagueHeat& league) const;
  
  /*load a population of pokemon and their rankings from a filepath */
  bool loadTeamPopulation();

  bool saveTeamPopulation();

  LeagueHeat constructLeague() const;
  void digestGame(GameHeat& gameHeat, LeagueHeat& league) const;

  void testInitialized() const;
public:

};

#endif /* TRAINER_H */