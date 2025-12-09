/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   old_trainer.h
 * Author: drendleman
 *
 * Created on September 17, 2020, 1:00 PM
 */

#ifndef OLD_TRAINER_H
#define OLD_TRAINER_H

#include "pokemonai/pkai.h"

#include <stdint.h>
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <boost/program_options.hpp>

#include "true_skill.h"
#include "ranked.h"

#include "ranked_team.h"
#include "ranked_evaluator.h"

class Type;
class Game;
class Evaluator;

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

class Trainer {
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

    /* if a team population is to be loaded to memory from a directory, this is where it is */
    std::string teamPath = "teams";

    /* if value is nonzero, the number of generations between writeOuts to disk. Otherwise, do not write out */
    size_t writeOutEvery = 0;

    /* minimum amount of time to work on a given league, in seconds */
    double minimumWorkTime = 120;

    /* probability that a pokemon will undergo a mutation */
    double mutationProbability = 0.55;

    /* probability that two pokemon will crossover to create another */
    double crossoverProbability = 0.035;

    /* probability that an entirely new pokemon will spontaneously find its self in the population */
    double seedProbability = 0.035;

    /* sizes of the six populations aka "leagues" */
    std::array<size_t, 6> teamPopulationSize = {490, 240, 194, 150, 121, 106};

    /* do we allow teams to rank against teams of different leagues? Useful for small population */
    bool enforceSameLeague = false;

    boost::program_options::options_description options(
        const std::string& category="trainer configuration",
        std::string prefix = "");
  };

  Ranker(const Config& cfg = Config{});
  virtual ~Ranker();

  /* create all variables, prepare trainer for running */
  void initialize();

  TrainerResult run() const;


protected:
  Config cfg_;

  std::shared_ptr<TrueSkillFactory> trueSkillFactory_;

  size_t generationsCompleted;

  /* amount of heats performed in each league */
  std::array<size_t, 6> heatsCompleted;

  /* population of teams to be run on. 0 implies no work will be done on the league */
  std::array<std::vector<std::shared_ptr<RankedTeam> >, 6> leagues;

  /* population of evaluators to be run on. If networks is 0, this MUST be greater than 0. */
  std::vector<std::shared_ptr<RankedEvaluator>> evaluators;

  /* if performing ranking, trialTeam and/or trialNet are set */
  std::shared_ptr<RankedTeam> trialTeam;

  /* base engine configuration used for agents */
  std::shared_ptr<PkCU> agentEngine;

  /* game instance used for evaluation */
  std::shared_ptr<Game> game;

  /* select two parents, weighted by fitness */
  std::array<size_t, 2> selectParent_Roulette(const std::vector<const RankedTeam>& cLeague) const;

  /* generate an array of teams which are contained within the current team. Will always return an empty set if the team contains one pokemon */
  void findSubteams(TrueSkillTeam& cTeam, size_t iTeam);

  /* generates a random population from previous leagues, or from random functions if at single pokemon league */
  size_t seedRandomTeamPopulation(size_t iLeague, size_t targetSize);

  void spawnTeamChildren(size_t iLeague, size_t& numMutated, size_t& numCrossed, size_t& numSeeded);

  /* determine the league that should receive the next work cycle, giving precedence to lower leagues */
  size_t determineWorkingLeague() const;

  /* stochastically find a match of ideal skill for a given team. If enforceSameLeague is false, allow matches from nearby leagues */
  TrueSkillTeam findMatch( const TrueSkillTeam& oTeam );

  /* finds an evaluator, or returns NULL if we are to use the dumb evaluator */
  size_t findEvaluator();

  /* determine if a given ranked_team is already in the population, and if so, return its index. SIZE_MAX if not */
  size_t findInPopulation(size_t iLeague, uint64_t teamHash) const;

  /* return true if findInPopulation returns a value */
  bool isInPopulation(const RankedTeam& cRankTeam) const;

  /* destroy the elements with the lowest rank from the league */
  void shrinkTeamPopulation(size_t iLeague, size_t targetSize);

  /* calculates interesting things about the given league */
  void calculateDescriptiveStatistics(size_t iLeague, TrainerResult& cResult) const;

  /* print information about the top n members of league iLeague */
  void printLeagueStatistics(size_t iLeague, size_t numMembers, const TrainerResult& cResult) const;

  /*load a population of pokemon and their rankings from a filepath */
  bool loadTeamPopulation();

  bool saveTeamPopulation();

public:

  void setGauntletTeam(const TeamNonVolatile& cTeam);

  bool seedEvaluator(const Evaluator& _eval);

  bool seedTeam(const TeamNonVolatile& cTeam);

  /* begin evolution process */
  void evolve();
};


#endif /* OLD_TRAINER_H */

