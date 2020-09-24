#ifndef TRAINER_H
#define TRAINER_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <array>
#include <memory>
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

  std::unordered_map<Battlegroup::Hash, uint64_t> teamGameCount;

  std::vector<GameHeat> games;
};

class Ranker {
public:
  struct Config {
    /*verbosity of trainer */
    int verbosity = 0;

    /*when above 0, thread parallelism is used to invoke games*/
    size_t numThreads = 0;

    /* what do we intend for trainer to do? */
    uint32_t gameType = GT_OTHER_EVOTEAMS;

    /* number of generations to complete, maximum */
    size_t maxGenerations = 12;

    /* the minimum number of games per generation */
    size_t minGamesPerGeneration = 10;

    /* if a team population is to be loaded to memory from a directory, this is where it is */
    std::string teamPath = "teams";

    /* if value is nonzero, the number of generations between writeOuts to disk. Otherwise, do not write out */
    size_t writeOutEvery = 0;

    /* minimum amount of time to work on a given league, in seconds */
    double minimumWorkTime = 20;

    /* sizes of the six populations aka "leagues" */
    std::array<size_t, 6> teamPopulationSize = {490, 240, 194, 150, 121, 106};

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
  virtual ~Ranker();

  /* create all variables, prepare trainer for running */
  void initialize();

  TrainerResult run() const;

  /* plays minGamesPerGeneration games for every tsTeam within league */
  void runLeague(LeagueHeat& league) const;

  /* plays minGamesPerGeneration games with tsTeam within league*/
  void gauntlet(BattlegroupPtr& tsTeam, LeagueHeat& league) const;

  GameHeat singleGame(
      BattlegroupPtr& team_a,
      BattlegroupPtr& team_b) const;
  
protected:
  Config cfg_;

  std::shared_ptr<GameFactory> gameFactory_;

  /* population of teams to be run on. 0 implies no work will be done on the league */
  TeamLeague teams;

  /* population of evaluators to be run on. If networks is 0, this MUST be greater than 0. */
  EvaluatorLeague evaluators;

  /* game instance used for evaluation, per thread */
  mutable std::vector<std::shared_ptr<Game> > games;

  /* generate an array of teams which are contained within the current team. Will always return an empty set if the team contains one pokemon */
  void findSubteams(TrueSkillTeam& cTeam, size_t iTeam);

  /* stochastically find a match of ideal skill for a given team. If enforceSameLeague is false, allow matches from nearby leagues */
  BattlegroupPtr findMatch(const BattlegroupPtr& oTeam, const LeagueHeat& league) const;

  /* calculates interesting things about the given league */
  void calculateDescriptiveStatistics(size_t iLeague, TrainerResult& cResult) const;

  /* print information about the top n members of league iLeague */
  void printLeagueStatistics(size_t iLeague, size_t numMembers, const TrainerResult& cResult) const;
  
  /*load a population of pokemon and their rankings from a filepath */
  bool loadTeamPopulation();

  bool saveTeamPopulation();

  void digestGame(GameHeat& gameHeat) const;
  
public:

  void setGameFactory(const std::shared_ptr<GameFactory>& gameFactory);
  void setGameFactory(const GameFactory& gameFactory) {
    return setGameFactory(std::make_shared<GameFactory>(gameFactory));
  }

  void setGame(const Game& game);

  bool addEvaluator(const Evaluator& _eval);

  bool addTeam(const TeamNonVolatile& cTeam);

};

#endif /* TRAINER_H */
