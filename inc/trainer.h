/* 
 * File:   trainer.h
 * Author: drendleman
 *
 * Created on September 22, 2020, 12:54 PM
 */

#ifndef TRAINER_H
#define TRAINER_H

#include "ranker.h"

#include "team_factory.h"

class Trainer : public Ranker {
public:
  struct Config : public Ranker::Config {
    /* number of generations to complete, maximum */
    size_t maxGenerations = 12;

    /* probability that a pokemon will undergo a mutation */
    double mutationProbability = 0.35;

    /* probability that two pokemon will crossover to create another */
    double crossoverProbability = 0.10;

    /* probability that an entirely new pokemon will spontaneously find its self in the population */
    double seedProbability = 0.05;

    /* sizes of the six populations aka "leagues" */
    std::vector<int64_t> teamPopulationSize = {30, 15, 10, 8, 6, 5};

    /* if value is nonzero, the number of generations between writeOuts to disk. Otherwise, do not write out */
    size_t writeOutEvery = 0;

    boost::program_options::options_description options(
        const std::string& category="trainer configuration",
        std::string prefix = "");

    Config() : Ranker::Config() {}
  };

  Trainer(const Config& cfg);

  virtual void initialize() override;

  /* begin evolution process */
  LeagueHeat evolve() const;
protected:
  Config cfg_;

  TeamFactory teamFactory_;

  double doNothingProbability_;

  void evolveGeneration(LeagueHeat& league) const;
  void resetLeague(LeagueHeat& league) const;

  virtual LeagueHeat constructLeague() const override;

  /* generates a random population from previous leagues, or from random functions if at single pokemon league */
  size_t seedRandomTeamPopulation(League& league) const;

  TeamLeague spawnTeamChildren(League& league) const;

  /* destroy the elements with the lowest rank from the league */
  size_t shrinkPopulations(League& league, const LeagueCount& children) const;
};

#endif /* TRAINER_H */
