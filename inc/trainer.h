/* 
 * File:   trainer.h
 * Author: drendleman
 *
 * Created on September 22, 2020, 12:54 PM
 */

#ifndef TRAINER_H
#define TRAINER_H

#include "ranker.h"

class Trainer : public Ranker {
public:
  struct Config : public Ranker::Config {
    /* probability that a pokemon will undergo a mutation */
    double mutationProbability = 0.55;

    /* probability that two pokemon will crossover to create another */
    double crossoverProbability = 0.035;

    /* probability that an entirely new pokemon will spontaneously find its self in the population */
    double seedProbability = 0.035;

  };

  /* begin evolution process */
  void evolve();
protected:
  Config cfg_;

  /* generates a random population from previous leagues, or from random functions if at single pokemon league */
  size_t seedRandomTeamPopulation(size_t iLeague, size_t targetSize);

  void spawnTeamChildren(size_t iLeague, size_t& numMutated, size_t& numCrossed, size_t& numSeeded);

  /* destroy the elements with the lowest rank from the league */
  void shrinkTeamPopulation(size_t iLeague, size_t targetSize);
};

#endif /* TRAINER_H */
