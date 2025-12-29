#ifndef NEURALNET_RANKED_H
#define NEURALNET_RANKED_H

#include "pokemonai/pkai.h"

#include <vector>
#include <map>

#include "pokemonai/ranked.h"

#include "pokemonai/experienceNet.h"
#include "pokemonai/neuralNet.h"
#ifdef _DISABLETEMPORALDIFFERENCE
#include "pokemonai/backpropNet.h"
typedef backpropSettings networkSettings_t;
typedef backpropNet network_t;
#elif _DISABLETEMPORALTRACE
#include "pokemonai/backpropNet.h"
typedef backpropSettings networkSettings_t;
typedef backpropNet network_t;
#else
#include "pokemonai/temporalpropNet.h"
typedef temporalpropSettings networkSettings_t;
typedef temporalpropNet network_t;
#endif

class evaluator_featureVector;
class EnvironmentNonvolatile;
class RankedTeam;
class TrueSkillTeam;
class Game;
struct Turn;

class ranked_neuralNet: public Ranked
{
public:
#ifdef _DISABLETEMPORALDIFFERENCE
  static Game* rolloutGame;
  static std::vector<float> rolloutFitnesses;
#endif

private:
  /* Is passed to planner to perform evaluation, Performs TD learning on nNet */
  network_t bNet;

  /* a running sum of the network's experience. Not preserved past a save */
  experienceNet experience;

  /* neural network's hash value */
  uint64_t hash;
  /* accumulated update error since the last read */
  fpType totalMeanSquaredError;
  /* number of weight updates recorded since last read */
  fpType numUpdates;
  /* number of games played at the last jitter epoch */
  size_t lastJitterEpoch;

public:
  static const std::string header;

  static void initStatic(size_t numRollouts = 100);
  static void uninitStatic();

  static const evaluator_featureVector* getEvaluator(size_t numInputNeurons, size_t numOutputNeurons);

  ~ranked_neuralNet() { };
  ranked_neuralNet(
    const neuralNet& nNet = neuralNet(), 
    size_t generation = 0, 
    const networkSettings_t& cSettings = networkSettings_t::defaultSettings,
    const experienceNetSettings& eSettings = experienceNetSettings::defaultSettings,
    const trueSkillSettings& settings = trueSkillSettings::defaultSettings);
  
  static ranked_neuralNet generateRandom(
    const std::vector<size_t>& layerWidths, 
    const networkSettings_t& cSettings = networkSettings_t::defaultSettings,
    const experienceNetSettings& eSettings = experienceNetSettings::defaultSettings,
    const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  static ranked_neuralNet jitter_create(const ranked_neuralNet& parent, const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  void jitter(const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  fpType getMeanSquaredError() const;

  fpType getNumUpdates() const { return numUpdates; }

  void resetMeanSquaredError()
  {
    totalMeanSquaredError = 0.0;
    numUpdates = 0.0;
  };

  void resetRecord()
  {
    Ranked::resetRecord();

    resetMeanSquaredError();

    lastJitterEpoch = 0;
  };

  size_t gamesSinceJitter() const
  {
    return getNumGamesPlayed() - lastJitterEpoch;
  };

  /* update two neural network rankings with TD */
  size_t update(const Game& cGame, const TrueSkillTeam& cTeam, size_t iTeam, bool updateWeights = true);

  /* performs monte-carlo backpropagation on bNet. Returns the error if an update was performed, or 0.0 if none */
  /* performs temporal difference learning on bNet. Returns the error if an update was performed, or 0.0 if none */
  static void propagate(
    const std::vector<Turn>& turns, 
    const EnvironmentNonvolatile& envNV,
    ranked_neuralNet& rankedNet, 
    size_t iTeam);

  /* updates the experience vector in the ranked_object based upon the game's trace */
  static void updateExperience(
    const std::vector<Turn>& turns, 
    const EnvironmentNonvolatile& envNV,
    ranked_neuralNet& rankedNet, 
    size_t iTeam);

  void setNetwork(const neuralNet& cNet);

  const neuralNet& getNetwork() const { return bNet.getNeuralNet(); };

  const experienceNet& getExperience() const { return experience; };

  /* updates weights, records statistics about the update */
  void updateWeights();

  /* generate the hash */
  void generateHash();

  /* overwrites the name of this hash file with one defined by its hash */
  void defineName();

  /* returns a NEW ranked_neuralNet's evaluator type, initialized to include the ranked_neuralNet's network.
   * Returns NULL if no evaluator of this size exists.
  */
  evaluator_featureVector* getEvaluator() const;
  static evaluator_featureVector* getEvaluator(const neuralNet& cNet);

  /* returns true if this neuralNet_ranked can even seed an evaluator type */
  bool hasEvaluator() const;
  static bool hasEvaluator(const neuralNet& cNet);

  const std::string& getName() const { return bNet.getNeuralNet().getName(); };

  uint64_t hash() const { return hash; };

  void output(std::ostream& oFile, bool printHeader = true) const;

  bool input(const std::vector<std::string>& lines, size_t& iLine);

  friend std::ostream& operator <<(std::ostream& os, const ranked_neuralNet& tR);
};

std::ostream& operator <<(std::ostream& os, const ranked_neuralNet& tR);

#endif /* NEURALNET_RANKED_H */
