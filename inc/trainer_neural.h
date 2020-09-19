/* 
 * File:   neural_trainer.h
 * Author: drendleman
 *
 * Created on September 17, 2020, 11:34 AM
 */

#ifndef NEURAL_TRAINER_H
#define NEURAL_TRAINER_H

#include <memory>
#include <vector>

#include "old_trainer.h"
#include "ranked_neuralNet.h"

struct networkTrainerResult
{
  // averages: (rank, mean, stdDev, plies, wins, draws, games)
  std::array<fpType, 7> averages;
  // stdDevs:
  std::array<fpType, 7> stdDevs;
  // minimums:
  std::array<fpType, 7> mins;
  // maximums:
  std::array<fpType, 7> maxes;
  // meansquared error: (average, stdDev, min, max)
  std::array<fpType, 4> meanSquaredError;
};

class TrainerNeural : public Trainer {
public:
  using base_t = Trainer;

  struct Config : public base_t::Config {
    /* backpropagation and temporal-difference settings used for all neural network training calculations */
    networkSettings_t netSettings;

    /* settings for each network's persistent experience table */
    experienceNetSettings expSettings;

    /* if a network population is to be loaded to memory from a directory, this is where it is */
    std::string networkPath = "networks";

    /* probability that an entirely new network will be seeded */
    double seedNetworkProbability;

    /* jitters the network after epoch games has been completed */
    size_t jitterEpoch = 2500;

    /* number of random rollouts to be performed by monte-carlo simulation */
    size_t numRollouts = 1000;

    /* size of the network population */
    size_t networkPopulationSize;

    /* the topology of newly created networks */
    std::vector<size_t> networkLayerSize;
  };


  void initialize();

protected:
  void calculateDescriptiveStatistics(networkTrainerResult& cResult) const;

  size_t seedRandomNetworkPopulation(size_t targetSize);

  void spawnNetworkChildren(size_t& numMutated, size_t& numCrossed, size_t& numSeeded);

  /* determine if a given ranked_neuralNet is already in the network list, and if so, return its index. SIZE_MAX if not */
  size_t findInNetworks(uint64_t teamHash) const;

  /* return true if findInNetworks returns a value */
  bool isInNetworks(const ranked_neuralNet& cRankNet) const;

  void shrinkNetworkPopulation(size_t targetSize);

  void printNetworkStatistics(size_t numMembers, const networkTrainerResult& cResult) const;

  bool loadNetworkPopulation();

  bool saveNetworkPopulation();

  void setGauntletNetwork(const neuralNet& cNet);
  bool seedNetwork(const neuralNet& cNet);

  /* population of networks to be run on. May possibly be 0 */
  std::vector<std::shared_ptr<ranked_neuralNet> > networks;

  std::shared_ptr<ranked_neuralNet> trialNet;
};

#endif /* NEURAL_TRAINER_H */

